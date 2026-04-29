# playground/resource/model/cornell_box 需求

## 1. 目标
本目录保存 Cornell Box 场景分件模型，当前属于正式注册场景的在用输入资产。

## 2. 当前使用方
- 已注册场景：
  - `PathTracingScene`
  - `RTRTScene`

## 3. 功能需求
- 应保留墙面、灯、长短盒等分件 OBJ/MTL 文件。
- 分件命名应清晰表达各个几何部件职责。
- 各部件文件名应与 `playground/config.pb.txt` 中 `cornell_box_*` 模型项一一对应。
