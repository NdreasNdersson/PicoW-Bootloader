# PicoW-Bootloader
The memory layout:

```
+=============+====================+=======+
| AREA        | CONTENT            | SIZE  |
+=============+====================+=======+
| Bootloader  |                    | 32k   |
+-------------+--------------------+-------+
| App Info    |                    | 4k    |
+-------------+--------------------+-------+
|             | App hash (32)      |       |
+-------------+--------------------+-------+
|             | Swap app hash (32) |       |
+-------------+--------------------+-------+
|             | App size (4)       |       |
+-------------+--------------------+-------+
|             | Swap app size (4)  |       |
+-------------+--------------------+-------+
|             | App downloaded (4) |       |
+-------------+--------------------+-------+
|             | App backed up (4)  |       |
+-------------+--------------------+-------+
|             | padding (4016)     |       |
+-------------+--------------------+-------+
| App Storage |                    | 4k    |
+-------------+--------------------+-------+
| App         |                    | 1004k |
+-------------+--------------------+-------+
```
