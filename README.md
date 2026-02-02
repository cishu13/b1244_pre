# Village Placer

A C++ tool for procedurally placing villages on a tile-based map with biome-specific parameters. This tool takes existing tile and biome data and strategically places villages while respecting terrain constraints and maintaining minimum distances between settlements.

## Requirements

- C++17 compatible compiler
- Libraries (included in the `libraries` folder):
  - nlohmann/json (JSON parsing)
  - stb_image_write.h (PNG image generation)

## Compilation

```bash
g++ -std=c++17 village_placer.cpp -o village_placer
```

## Usage

The program now accepts the biomes CSV file path as a parameter:

```bash
./village_placer <tiles.csv> <params.json> <biomes.csv> <output.csv> [output.png]
```

### Example:

```bash
./village_placer inputs/tiles.csv inputs/params.json inputs/biomes.csv outputs/tiles_with_villages.csv outputs/map_with_villages.png
```

## Input File Formats

### Tiles CSV
A grid of tile types where each cell contains a string identifier (e.g., "grass", "forest", "mountain", "water").

### Biomes CSV
A grid matching the dimensions of the tiles CSV, where each cell contains a biome identifier (e.g., "plains", "forest", "desert").

### Parameters JSON
Configuration file with the following structure:

```json
{
  "tile_colors": {
    "grass": "#7CFC00",
    "forest": "#228B22",
    "mountain": "#A0522D",
    "water": "#1E90FF",
    "desert": "#F4A460"
  },
  "village_params": {
    "plains": {
      "village_count": 10,
      "min_distance": 15,
      "min_cluster_size": 4,
      "max_cluster_size": 8,
      "seed": 12345
    },
    "forest": {
      "village_count": 5,
      "min_distance": 20,
      "min_cluster_size": 3,
      "max_cluster_size": 6,
      "seed": 54321
    }
  }
}
```

## Output

- **CSV Output**: Updated tile grid with "village" tiles added
- **PNG Visualization**: Color-coded map with villages shown in purple (#800080)

## Algorithm

1. For each biome specified in the parameters:
   - Find valid tiles (non-mountain, non-water, non-village) within that biome
   - Place village centers with minimum Manhattan distance between them
   - For each village center, create a cluster of 4-10 connected "village" tiles

## License

This project uses:
- [nlohmann/json](https://github.com/nlohmann/json) - MIT License
- [stb_image_write.h](https://github.com/nothings/stb) - Public Domain / MIT License










