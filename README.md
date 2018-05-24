# GetMaterialAndObj插件使用说明

## 使用说明
-放入Plugins文件夹下，重新生成下项目

- 两个可输入文本框，确定material的尺寸大小
- GetMaterial按钮可以实现对于选中模型材质的导出
- Material对应放在`/FileName/Content/GetObj/maps/`文件夹下
- 可导出选中模型的所有Material
- GetObjAndMtl按钮可以实现对于选中模型Obj和对应mtl的导出
- Obj和mtl文件放在`/FileName/Content/GetObj/`文件夹下，并以统一名字命名
- 可以实现对于多LOD级别的模型导出，会在命名最后加入LODN加以区别

-------------------
## 4.18版本修改为4.19版本
由于Unreal内部对于类的重新封装，需要修改以下几个部分代码，才能运行
GetMaterialAndObj.cpp
4.18     | 4.19
-------- | -------- 
260行`check(VertexCount == RenderData.VertexBuffer.GetNumVertices());`|`check(VertexCount == RenderData.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());`
266行`const FVector& OSPos = RenderData.PositionVertexBuffer.VertexPosition(i);`|`const FVector& OSPos = RenderData.VertexBuffers.PositionVertexBuffer.VertexPosition(i);`
279行`const FVector2D UV = RenderData.VertexBuffer.GetVertexUV(i, 0);`|`const FVector2D UV = RenderData.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 0);`
289行`const FVector& OSNormal = RenderData.VertexBuffer.VertexTangentZ(i);`|`const FVector& OSNormal = RenderData.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(i);`