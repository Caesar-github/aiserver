1. 将src/vendor/CMakeList.txt中option(ENABLE_SAMPLE_NODE_ROCKX  "enable sample node rockx" OFF)开打
2. 将oem/usr/share/aiserver/aicamera.json中node_4、node_11的node_name:rockx修改为rockxdemo。
3. 删除node_12, node_13,并将json文件最下方包含node_12和node_13的link配置删除。
4. 通过修改node_4、node_11中的opt_rockx_model，可以替换算法模型进行效果验证，目前可选模型包括。
-- rockx_head_detect
-- rockx_face_detect_v2
-- rockx_face_detect_v3
-- rockx_face_detect_v3_large
5. 若运行时提示 xxx model data not found, 需将对应模型文件从external/rockx/sdk/rockx-data-rv1109/目录下拷贝到/usr/lib目录下。
6. 其他rockx算法添加，可参考samples/filter/rockx下的文件进行开发集成。
