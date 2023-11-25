# Barcode

## Understanding Infrared Sensor
Component Pin | Function                                                            |
|-------------|---------------------------------------------------------------------|
| D0          | Turns output signal to HIGH when it doesn’t receive enough infrared |

## Block Diagram
![barcode block](../../images/driver_barcode/driver_bar_block.png)

## Flowchart
### Decode Barcode Char
![barcode_flow_1](../../images/driver_barcode/driver_bar_flow.png)

### Determine bits
  - Uses [Decode Barcode Char](#decode-barcode-char)
![barcode_flow_2](../../images/driver_barcode/driver_bar_flow_2.png)
