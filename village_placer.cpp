// village_placement.cpp
// ---------------------
// Load tiles.csv and place villages randomly on valid tiles (non-mountain, non-water, non-village)
// with minimum Manhattan distance between village clusters.
// Each village changes 4–10 connected tiles to village type.
// Save updated grid to CSV and create visualization PNG (using stb_image_write).

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "libraries/json.hpp"     // https://github.com/nlohmann/json
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libraries/stb_image_write.h"   // https://github.com/nothings/stb

using json = nlohmann::json;
using TileGrid = std::vector<std::vector<std::string>>;
using Coord = std::pair<int,int>;

// --- File I/O ----------------------------------------------------------------

json load_json(const std::string &path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Unable to open JSON: " + path);
    json j; f >> j;
    return j;
}

TileGrid load_csv(const std::string &path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Unable to open CSV: " + path);
    TileGrid grid;
    std::string line;
    while (std::getline(f, line)) {
        std::vector<std::string> row;
        size_t start = 0;
        while (start < line.size()) {
            auto comma = line.find(',', start);
            if (comma == std::string::npos) comma = line.size();
            row.push_back(line.substr(start, comma - start));
            start = comma + 1;
        }
        grid.push_back(std::move(row));
    }
    return grid;
}

void write_csv(const TileGrid &grid, const std::string &path) {
    std::ofstream f(path);
    for (auto &row : grid) {
        for (size_t i=0; i<row.size(); ++i) {
            f << row[i] << (i+1<row.size() ? "," : "");
        }
        f << "\n";
    }
}

void write_png_with_villages(const TileGrid &grid,
                             const std::unordered_map<std::string,std::string> &colors,
                             const std::string &path) {
    int h = grid.size();
    int w = grid.empty() ? 0 : grid[0].size();
    std::vector<unsigned char> img(w * h * 3);
    // build palette
    std::unordered_map<std::string, std::array<unsigned char,3>> palette;
    for (auto &p : colors) {
        auto &hex = p.second;
        unsigned r = std::stoul(hex.substr(1,2), nullptr, 16);
        unsigned g = std::stoul(hex.substr(3,2), nullptr, 16);
        unsigned b = std::stoul(hex.substr(5,2), nullptr, 16);
        palette[p.first] = { (unsigned char)r, (unsigned char)g, (unsigned char)b };
    }
    palette["village"] = {128,0,128};

    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            auto it = palette.find(grid[y][x]);
            std::array<unsigned char,3> c = it!=palette.end()
                ? it->second
                : std::array<unsigned char,3>{0,0,0};
            int idx = (y*w + x)*3;
            img[idx]   = c[0];
            img[idx+1] = c[1];
            img[idx+2] = c[2];
        }
    }
    stbi_write_png(path.c_str(), w, h, 3, img.data(), w*3);
}

// --- Village Placement Logic ------------------------------------------------

std::vector<Coord> get_valid_tiles(const TileGrid &tiles) {
    std::vector<Coord> valid;
    int h = tiles.size(), w = tiles.empty() ? 0 : tiles[0].size();
    for (int y=0; y<h; ++y)
        for (int x=0; x<w; ++x) {
            const auto &t = tiles[y][x];
            if (t!="mountain" && t!="water" && t!="village")
                valid.emplace_back(x,y);
        }
    return valid;
}

int manhattan_distance(const Coord &a, const Coord &b) {
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

std::vector<Coord> create_village_cluster(TileGrid &tiles,
                                          const Coord &center,
                                          int cluster_size,
                                          std::mt19937 &rng) {
    int h = tiles.size(), w = tiles[0].size();
    std::vector<Coord> changed;
    auto [cx,cy] = center;
    auto valid_type = [&](int x,int y){
        auto &t = tiles[y][x];
        return t!="mountain" && t!="water" && t!="village";
    };
    if (cx>=0 && cx<w && cy>=0 && cy<h && valid_type(cx,cy)) {
        tiles[cy][cx] = "village";
        changed.push_back(center);
    }
    std::vector<Coord> candidates = changed;
    std::uniform_int_distribution<size_t> dist;
    std::array<Coord,4> dirs{{{0,1},{1,0},{0,-1},{-1,0}}};
    while ((int)changed.size()<cluster_size && !candidates.empty()) {
        std::uniform_int_distribution<size_t> pick(0, candidates.size()-1);
        size_t idx = pick(rng);
        Coord curr = candidates[idx];
        candidates.erase(candidates.begin()+idx);
        for (auto &d : dirs) {
            int nx = curr.first + d.first;
            int ny = curr.second + d.second;
            if (nx>=0 && nx<w && ny>=0 && ny<h && valid_type(nx,ny) 
                && (int)changed.size()<cluster_size) {
                tiles[ny][nx] = "village";
                changed.emplace_back(nx,ny);
                candidates.emplace_back(nx,ny);
            }
            if ((int)changed.size()>=cluster_size) break;
        }
    }
    return changed;
}

std::vector<Coord> place_villages_by_biome(TileGrid &tiles,
                                           const json &village_params,
                                           const std::string &biomes_csv_path) {
    // load biomes same CSV split logic
    TileGrid biome = load_csv(biomes_csv_path);
    int h=tiles.size(), w=tiles[0].size();

    std::vector<Coord> all_placed;
    std::random_device rd;

    for (auto &el : village_params.items()) {
        std::string biome_name = el.key();
        auto params = el.value();
        int count = params.value("village_count",0);
        int min_dist = params.value("min_distance",15);
        int min_cl = params.value("min_cluster_size",4);
        int max_cl = params.value("max_cluster_size",10);
        unsigned seed = params.value("seed", rd());
        if (count<=0) continue;

        std::mt19937 rng(seed);
        std::vector<Coord> valid;
        for (int y=0; y<h; ++y)
            for (int x=0; x<w; ++x)
                if (tiles[y][x]!="mountain" && tiles[y][x]!="water" && tiles[y][x]!="village"
                    && biome[y][x]==biome_name)
                    valid.emplace_back(x,y);

        std::vector<Coord> placed;
        int attempts = 0, max_attempts = count*100;
        while ((int)placed.size()<count && attempts<max_attempts && !valid.empty()) {
            ++attempts;
            std::uniform_int_distribution<size_t> pick(0, valid.size()-1);
            Coord cand = valid[pick(rng)];
            bool too_close = false;
            for (auto &c0 : all_placed) if (manhattan_distance(c0,cand)<min_dist) { too_close=true; break; }
            for (auto &c1 : placed)    if (manhattan_distance(c1,cand)<min_dist) { too_close=true; break; }
            if (too_close) continue;

            int cluster_size = std::uniform_int_distribution<int>(min_cl, max_cl)(rng);
            auto changed = create_village_cluster(tiles, cand, cluster_size, rng);
            if (!changed.empty()) {
                placed.push_back(cand);
                // remove changed from valid
                std::unordered_set<size_t> to_erase;
                for (auto &ch : changed)
                    for (size_t i=0;i<valid.size();++i) if(valid[i]==ch) to_erase.insert(i);
                std::vector<Coord> new_valid;
                for (size_t i=0;i<valid.size();++i) if(!to_erase.count(i)) new_valid.push_back(valid[i]);
                valid.swap(new_valid);
            }
        }
        all_placed.insert(all_placed.end(), placed.begin(), placed.end());
        std::cout << "Placed "<<placed.size()<<" in biome "<<biome_name<<"\n";
    }
    return all_placed;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr<<"Usage: village_placement <tiles.csv> <params.json> <biomes.csv> <output.csv> [output.png]\n";
        return 1;
    }
    std::string tiles_csv = argv[1];
    std::string params_json = argv[2];
    std::string biomes_csv = argv[3];
    std::string output_csv = argv[4];
    std::string output_png = (argc>5 ? argv[5] : std::string("villages.png"));

    auto tiles = load_csv(tiles_csv);
    auto params = load_json(params_json);
    auto villages = place_villages_by_biome(tiles, params["village_params"], biomes_csv);

    write_csv(tiles, output_csv);
    write_png_with_villages(tiles, params["tile_colors"].get<std::unordered_map<std::string,std::string>>(), output_png);
    std::cout<<"Successfully placed "<<villages.size()<<" villages.\n";
    return 0;
}

