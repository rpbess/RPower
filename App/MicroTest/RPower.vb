Imports MicronetBus.Lib

Public Class RPower
    Public Property Address As String = ""
    Public Property Name As String = ""
    Public LSM303 As New LSM303Sensor

    Private _bus As Micronet = Nothing
    Private _finded As Boolean = False

    Sub New(bus As Micronet)
        _bus = bus
    End Sub

    Public Sub Run()
        Dim th As New Threading.Thread(AddressOf WaitBoardInfo)
        th.IsBackground = True
        th.Start()
    End Sub

    Public Sub Beep(pulse As UInt16)
        If _finded = False Then Return
        If pulse > 3000 Then pulse = 3000
        Dim request = New Micronet.MicronetFrame
        request.Cmd = 3
        request.TargetAddress = Address
        request.Data = {pulse >> 8, pulse And &HFF}
        Dim response = _bus.Request(request, 1000)
    End Sub

    Public Sub SetLedState(state As Boolean)
        If _finded = False Then Return
        Dim request = New Micronet.MicronetFrame
        request.Cmd = 4
        request.TargetAddress = Address
        request.Data = {If(state, 1, 0)}
        Dim response = _bus.Request(request, 1000)
    End Sub

    Public Function GetInputVoltage() As Double
        If _finded = False Then Return -1
        Dim request = New Micronet.MicronetFrame
        request.Cmd = 5
        request.TargetAddress = Address
        Dim response = _bus.Request(request, 1000)
        If response.State = Micronet.MicronetFrame.FrameState.OK And response.Data.Length >= 2 Then
            Return (response.Data(0) * 256 + response.Data(1)) * 0.001
        Else
            Return -1
        End If
    End Function

    Public Sub SetFetState(fet As Byte, state As Boolean)
        If _finded = False Then Return
        Dim request = New Micronet.MicronetFrame
        request.Cmd = 6
        request.TargetAddress = Address
        request.Data = {fet, If(state, 1, 0)}
        Dim response = _bus.Request(request, 1000)
        If response.State <> Micronet.MicronetFrame.FrameState.OK Then Throw New Exception("the request was not successful")
    End Sub

    Private Sub WaitBoardInfo()
        Dim waitCount = 0
        While Address.Length = 0
            If waitCount = 0 Then
                waitCount = 10
                _bus.FindRequest()
            End If
            waitCount -= 1
            Threading.Thread.Sleep(1000)
            For Each deviceId In _bus.Devices.Keys
                If _bus.Devices(deviceId).ToLower.Contains("rpower") Then
                    Address = deviceId
                    Name = _bus.Devices(deviceId)
                End If
            Next
        End While
        _finded = True
        Beep(50)
        Threading.Thread.Sleep(100)
        Beep(50)
        Threading.Thread.Sleep(100)
        Beep(50)
    End Sub

    Public Function ReadLsm303() As LSM303Sensor
        Dim lsm As New LSM303Sensor
        If _finded = False Then Return lsm
        Dim request = New Micronet.MicronetFrame
        request.Cmd = 2
        request.TargetAddress = Address
        Dim response = _bus.Request(request, 1000)
        If response.State = Micronet.MicronetFrame.FrameState.OK And response.Data.Length = 12 Then
            lsm.Accelerometer.X = (response.Data(0) * 256 + response.Data(1) - 16000) / 1000.0
            lsm.Accelerometer.Y = (response.Data(2) * 256 + response.Data(3) - 16000) / 1000.0
            lsm.Accelerometer.Z = (response.Data(4) * 256 + response.Data(5) - 16000) / 1000.0
            lsm.Magnetometer.X = (response.Data(6) * 256 + response.Data(7) - 16000) / 1000.0
            lsm.Magnetometer.Y = (response.Data(8) * 256 + response.Data(9) - 16000) / 1000.0
            lsm.Magnetometer.Z = (response.Data(10) * 256 + response.Data(11) - 16000) / 1000.0
        End If
        Return lsm
    End Function

    Public Sub PowerReset()
        If _finded = False Then Return
        Dim request = New Micronet.MicronetFrame
        request.Cmd = 7
        request.TargetAddress = Address
        Dim response = _bus.Request(request, 1000)
    End Sub

    Public Function ReadHTS221() As HTS221
        Dim sensor As New HTS221
        If _finded = False Then Return sensor
        Dim request = New Micronet.MicronetFrame
        request.Cmd = 1
        request.TargetAddress = Address
        Dim response = _bus.Request(request, 1000)
        If response.State = Micronet.MicronetFrame.FrameState.OK And response.Data.Length = 2 Then
            sensor.Humidity = response.Data(0)
            sensor.Temperature = response.Data(1) - 127
        End If
        Return sensor
    End Function

    Public Class HTS221
        Public Property Temperature As Int16 = 0
        Public Property Humidity As UInt16 = 0
    End Class

    Public Class LSM303Sensor
        Public Property Accelerometer As New LsmDataContainer
        Public Property Magnetometer As New LsmDataContainer
        Public Class LsmDataContainer
            Public Property X As Double = 0.0
            Public Property Y As Double = 0.0
            Public Property Z As Double = 0.0
        End Class
    End Class
End Class
