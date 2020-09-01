//
//  Const.swift
//  BLECar
//
//  Created by Ryota Ayaki on 2020/08/30.
//  Copyright © 2020 Ryota Ayaki. All rights reserved.
//

let kPeripheralName = "Bluefruit"
let kServiceUUID: String = "EE02AC5B-32A0-0CDD-DB39-5D3AB4336C6D"
let kCharacteristicUUID: String = "EE0234A7-32A0-0CDD-DB39-5D3AB4336C6D"
let kBuzzerByteArray:   [UInt8] = [0x00, 0x00, 0x00, 0x00, 01]
let kForwardByteArray:  [UInt8] = [0x01, 0xFF, 0x01, 0xFF, 00]
let kBackByteArray:     [UInt8] = [0x00, 0xFF, 0x00, 0xFF, 00]
let kStopByteArray:     [UInt8] = [0x00, 0x00, 0x00, 0x00, 00]
let kLeftByteArray:     [UInt8] = [0x00, 0xFF, 0x01, 0xFF, 00]
let kRightByteArray:    [UInt8] = [0x01, 0xFF, 0x00, 0xFF, 00]
