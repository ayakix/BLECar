//
//  ViewController.swift
//  BLECar
//
//  Created by Ryota Ayaki on 2020/08/30.
//  Copyright © 2020 Ryota Ayaki. All rights reserved.
//

import UIKit
import JoystickView

class ViewController: UIViewController {
    @IBOutlet weak var hJoystickView: JoystickView!
    @IBOutlet weak var vJoystickView: JoystickView!
    private let kDirectionThreshold: Float = 0.3
    private let kShortestCommandTimeInterval = 0.1
    private var bluetoothService: BluetoothService?
    private var lastDirection: JoystickMoveDriection = .none
    private var lastCommandDate: Date = Date()
    private var workItem: DispatchWorkItem?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        hJoystickView.delegate = self
        hJoystickView.form = .horizontal
        vJoystickView.delegate = self
        vJoystickView.form = .vertical
        
        initBluetooth()
    }
    
    @IBAction private func refreshButtonClicked(_ sender: UIButton) {
        initBluetooth()
    }
    
    @IBAction func buzzerButtonClicked(_ sender: UIButton) {
        bluetoothService?.write(byteArray: kBuzzerByteArray)
    }
}

private extension ViewController {
    func initBluetooth() {
        bluetoothService = BluetoothService()
        bluetoothService?.setupBluetoothService(peripheralName: kPeripheralName, serviceUUID: kServiceUUID, characteristicUUID: kCharacteristicUUID)
    }
    
    func canSendCommand() -> Bool {
        return lastCommandDate.timeIntervalSinceNow < -1 * kShortestCommandTimeInterval
    }
    
    func getByteArray(direction: JoystickMoveDriection) -> [UInt8] {
        switch direction {
        case .up:
            return kForwardByteArray
        case .down:
            return kBackByteArray
        case .left:
            return kLeftByteArray
        case .right:
            return kRightByteArray
        default:
            return kStopByteArray
        }
    }
}

extension ViewController: JoystickViewDelegate {
    func joystickView(_ joystickView: JoystickView, didMoveto x: Float, y: Float, direction: JoystickMoveDriection) {
//        print("joystick move to x:\(x) y:\(y) direction:\(direction.rawValue)")
        guard canSendCommand(), lastDirection != direction else {
            return
        }
        workItem?.cancel()
        
        var overThreshold = false
        switch direction {
        case .up:
            overThreshold = y > kDirectionThreshold
        case .down:
            overThreshold = y < -1 * kDirectionThreshold
        case .right:
            overThreshold = x > kDirectionThreshold
        case .left:
            overThreshold = x < -1 * kDirectionThreshold
        default:
            break
        }
        
        if overThreshold {
            print(direction)
            bluetoothService?.write(byteArray: getByteArray(direction: direction))
            lastDirection = direction
            lastCommandDate = Date()
        }
    }
    
    func joystickViewDidEndMoving(_ joystickView: JoystickView) {
        guard canSendCommand() else {
            workItem = DispatchWorkItem() { self.joystickViewDidEndMoving(joystickView) }
            DispatchQueue.main.asyncAfter(deadline: .now() + kShortestCommandTimeInterval, execute: self.workItem!)
            return
        }
        
        let direction: JoystickMoveDriection = .none
        if lastDirection != direction {
            print(direction)
            bluetoothService?.write(byteArray: getByteArray(direction: direction))
            lastDirection = direction
            lastCommandDate = Date()
        }
    }
}
