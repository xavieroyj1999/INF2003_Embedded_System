# Partial Integration
- [Straight Path](#straight-path-flowchart)
- [Right/left-angle turn](#rightleft-angle-turn-flowchart)
- [Drive through and detect barcode-decode and send to WiFi](#drive-through-and-detect-barcode-decode-and-send-to-wifi-flowchart)
- [Drive through and detect obstacles and Stop](#drive-through-and-detect-obstacles-and-stop-flowchart)

## Straight Path Flowchart
  - Uses [Infrared Interrupt](../partial_integration_include#left-right-front-infrared-interrupt-flowchart) | [Encoder Interrupt](../driver/encoder/README.md#flowchart)
![straight_flow](../images/partial/partial_straight_flow.png)

## Right/left-angle turn Flowchart
  - Uses [Straight Path](#straight-path-flowchart) | [Infrared Interrupt](../partial_integration_include#left-right-front-infrared-interrupt-flowchart) | [Encoder Interrupt](../driver/encoder/README.md#flowchart)
![turn_flow](../images/partial/partial_turn_flow.png)

## Drive through and detect barcode-decode and send to WiFi Flowchart
  - Uses [Straight Path](#straight-path-flowchart) | [Infrared Interrupt](../partial_integration_include#left-right-front-infrared-interrupt-flowchart) | [Encoder Interrupt](../driver/encoder/README.md#flowchart) | [Wi-Fi](../driver/wifi/README.md) | [Barcode](../driver/barcode/README.md#flowchart)
    
![barcode_wifi_flow](../images/partial/partial_barcode_flow.png)

## Drive through and detect obstacles and Stop flowchart
  - Uses [Straight Path](#straight-path-flowchart) | [Ultrasonic Interrupt](../driver/ultrasonic/README.md#flowchart)
![ultra_detect_flow](../images/partial/partial_ultra_flow.png)
