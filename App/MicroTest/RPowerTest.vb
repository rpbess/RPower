Imports MicronetBus.Lib

Module RpowerTest
    Sub Main()
        Dim _bus As New Micronet("COM19", 115200)
        _bus.Open()
        _bus.RunScanner()
        Dim rpower As New RPower(_bus)
        rpower.Run()
        While True
            Console.Clear()
            Dim lsm = rpower.ReadLsm303
            Dim th = rpower.ReadHTS221()
            Console.WriteLine(rpower.Name + " [" + rpower.Address + "]")
            Console.WriteLine("T=" + th.Temperature.ToString + "   H=" + th.Humidity.ToString)
            Console.WriteLine("Acc: " + lsm.Accelerometer.X.ToString + " " + lsm.Accelerometer.Y.ToString + " " + lsm.Accelerometer.Z.ToString + " ")
            Console.WriteLine("Input voltage: " + rpower.GetInputVoltage.ToString)
            If th.Temperature > 33 Then rpower.SetFetState(2, True)
            If th.Temperature < 30 Then rpower.SetFetState(2, False)
            Threading.Thread.Sleep(500)
        End While
    End Sub

End Module
