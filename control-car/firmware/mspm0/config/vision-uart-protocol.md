# K230 视觉 UART 协议

> 用途：规定庐山派 K230 / CanMV K230 与 MSPM0G3507 小车主控之间的串口数据格式。  
> 原则：K230 只输出视觉结果；MSPM0 负责运动控制、PID、状态机与安全保护。

---

## 1. 串口参数

```text
baudrate: 115200
format: 8N1
line ending: \n
encoding: ASCII / UTF-8
```

接线：

```text
K230 TXD → MSPM0 RX
K230 RXD → MSPM0 TX
K230 GND → MSPM0 GND
K230 5V  → 独立 5V 稳压输出
```

必须共地。不要把 5V TTL 信号直接接到 3.3V IO。

---

## 2. 帧格式

采用“一行一帧”的文本协议：

```text
VISION,KEY=VALUE,KEY=VALUE,...\n
```

优势：

- 串口助手可直接看懂；
- MSPM0 解析简单；
- 方便比赛现场手动排错；
- 比二进制协议更适合前期训练。

---

## 3. K230 → MSPM0 示例

```text
VISION,MODE=LINE,OK=1,CX=154,CY=102,ERR=-6,CONF=83
VISION,MODE=BLOB,OK=1,CX=121,CY=88,W=34,H=29,ID=RED,CONF=90
VISION,MODE=TAG,OK=1,ID=3,CX=118,CY=92,ROT=-12,CONF=95
VISION,MODE=QR,OK=1,DATA=START_A,CONF=99
VISION,MODE=YOLO,OK=1,ID=TARGET,CX=130,CY=76,W=42,H=30,CONF=88
VISION,MODE=NONE,OK=0,ERR=LOST,CONF=0
```

---

## 4. 字段定义

| 字段 | 类型 | 含义 |
|---|---|---|
| `VISION` | 固定帧头 | 表示这是 K230 视觉帧。 |
| `MODE` | 字符串 | `LINE` / `BLOB` / `TAG` / `QR` / `YOLO` / `NONE`。 |
| `OK` | 0/1 | 是否识别成功。 |
| `CX` | int | 目标中心 x 坐标。 |
| `CY` | int | 目标中心 y 坐标。 |
| `ERR` | int | 横向偏差，左负右正。 |
| `W` | int | 目标框宽度。 |
| `H` | int | 目标框高度。 |
| `ID` | string/int | 类别、颜色、AprilTag 编号或目标名。 |
| `ROT` | int | 旋转角度，单位先按度处理。 |
| `DATA` | string | 二维码内容或任务字符串。 |
| `CONF` | 0~100 | 置信度/稳定度。 |

字段可以缺省。MSPM0 解析时必须允许未知字段存在，不能因为新增字段导致崩溃。

---

## 5. MSPM0 最小结构体

```c
typedef struct {
    uint8_t ok;
    char mode[12];
    int16_t cx;
    int16_t cy;
    int16_t err;
    int16_t w;
    int16_t h;
    int16_t rot;
    int16_t conf;
    char id[16];
    char data[32];
    uint32_t last_update_ms;
} vision_result_t;
```

---

## 6. MSPM0 解析策略

1. UART 中断或 DMA 接收字符；
2. 放入 ring buffer；
3. 主循环按行取出；
4. 判断是否以 `VISION,` 开头；
5. 逐个解析 `KEY=VALUE`；
6. 更新 `vision_result_t`；
7. 刷新 `last_update_ms`；
8. OLED 显示当前视觉状态。

不要在 UART 中断里做复杂字符串解析。

---

## 7. 安全超时

建议超时策略：

| 条件 | MSPM0 动作 |
|---|---|
| `now - last_update_ms < 100ms` | 可使用视觉数据。 |
| `100ms ~ 300ms` | 视觉数据变旧，降低权重或减速。 |
| `> 300ms` | 视觉超时，禁止继续用旧视觉数据。 |
| 连续 `OK=0` | 降速、搜索或停车。 |
| `CONF` 低于阈值 | 不参与闭环，只显示状态。 |

视觉丢失时不能高速继续冲。

---

## 8. 融合灰度循迹

第一版灰度为主，K230 为辅：

```text
if line_sensor_ok:
    control_error = line_error

if line_sensor_ok && vision_ok && vision_conf >= 70:
    control_error = 0.8 * line_error + 0.2 * vision_error

if !line_sensor_ok && vision_ok && vision_conf >= 80:
    control_error = vision_error
    base_speed = low_speed

if !line_sensor_ok && !vision_ok:
    stop_or_search()
```

不要一开始就完全依赖视觉循迹。

---

## 9. 调试命令建议

MSPM0 人机串口可增加：

```text
vision show
vision mode line
vision mode blob
vision mode tag
vision conf 70
vision timeout 300
vision weight 0.2
```

返回示例：

```text
DATA: vision_mode=LINE,ok=1,cx=154,cy=102,err=-6,conf=83,age_ms=24
OK: vision_timeout=300
ERR: vision frame timeout
```

---

## 10. 验收标准

- K230 能每秒输出 10~30 行 `VISION,...`；
- MSPM0 能稳定解析，不乱码、不死机；
- OLED 能显示视觉模式、OK、ERR、CONF、AGE；
- 拔掉 K230 后 MSPM0 能超时报警；
- 视觉帧异常不会导致电机失控；
- 小车不接 K230 时仍能完成基础灰度循迹。
