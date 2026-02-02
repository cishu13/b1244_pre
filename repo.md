# Village Placer

## Repo简介

**Village Placer** 是一个基于 C++17 开发的程序化村庄放置工具，用于在瓦片网格（tile-based）游戏地图上智能生成村庄聚落。

**目标**：在给定的地形数据上，按照生物群系（biome）参数自动放置村庄，生成符合游戏设计需求的地图数据和可视化图像。

**解决的问题**：
- 在有效地形（排除山地、水域、已有村庄）上随机放置村庄中心点
- 使用曼哈顿距离确保村庄间保持合理间距，避免聚落过于密集
- 为每个村庄生成 4-10 个连通瓦片的集群结构
- 支持按生物群系（平原、森林、山地等）设置差异化的村庄数量、密度和大小参数

**技术栈**：
- **语言**：C++17
- **依赖库**：nlohmann/json（JSON 配置解析）、stb_image_write（PNG 可视化输出）
- **部署**：支持 Docker 容器化构建与运行

**项目结构**：
- `village_placer.cpp` - 主程序，包含村庄放置逻辑
- `inputs/` - 输入文件目录
  - `tiles.csv` - 瓦片网格数据
  - `biomes.csv` - 生物群系数据
  - `params.json` - 配置参数
- `libraries/` - 第三方库
  - `json.hpp` - nlohmann/json 库
  - `stb_image_write.h` - STB 图像写入库

程序读取瓦片 CSV、生物群系 CSV 和参数 JSON 配置文件，输出更新后的瓦片 CSV 和彩色可视化 PNG 地图。

## 题目Prompt

Modify village placement so that each cluster of villages contains 1 of: town center, shop, or monastery, on a random village tile. add to the params file to determine the weighted chance of each and distinct color. if there are more than 5 village tiles in a cluster, add a second, different tile.

## PR链接

待创建
