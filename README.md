HELLO THERE WELLCOME TO MY DIRECTX12 REPOSITORY 
In here i take you through how i am learning directx12

for starters the sample images below show the teapot that i drew using 16 point control patches, in both wire frame and solid.

> Current Features.
- It has support for toggling v-sync on and off to increase the frame rate.
- Supports full screen rendering.

> ## SAMPLE WIRE-FRAME
![wire frame](assets/wire-frame.png)

> ## SAMPLE-SOLID
![solid](assets/solid.png)

THE PROCESS<br>
```+-----------------------------+
| Initialization             |
| - Device, Swap Chain       |
| - Command Queue            |
| - Descriptor Heaps         |
| - Root Signature & PSO     |
| - Resource Uploads         |
+-----------------------------+

+-----------------------------+
| Frame Resources (per-frame)|
| - Command Allocator        |
| - Command List             |
| - Constant Buffers         |
| - Fence & Sync             |
+-----------------------------+

+-----------------------------+
| Scene Graph / ECS          |
| - Entities & Components    |
| - Transform Hierarchy      |
| - Material & Mesh Systems  |
+-----------------------------+

+-----------------------------+
| Render Loop                |
| - Update Logic             |
| - Record Command List      |
| - Resource Binding         |
| - Draw Calls               |
| - Execute & Present        |
+-----------------------------+
```
