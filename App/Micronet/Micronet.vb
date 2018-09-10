Imports System.IO.Ports

Public Class Micronet

    Private _serial As SerialPort = Nothing
    Private _evt As New Threading.AutoResetEvent(True)
    Private _frameBytes(10000) As Byte
    Private _frameBytesCursor As Integer = 0
    Private _currentFrame As New MicronetFrame
    Private _waitEvent As New Threading.AutoResetEvent(False)
    Public Event OnFrameReceive(Frame As MicronetFrame)
    Public Property Devices As New Dictionary(Of String, String)
    Private _locker As New Object

    Sub New(port As String, baud As Integer)
        _serial = New SerialPort(port, baud)
        AddHandler _serial.DataReceived, AddressOf DataReceivedHandler
    End Sub

    Public Sub RunScanner()
        Dim th As New Threading.Thread(AddressOf DeviceScanner)
        th.IsBackground = True
        th.Start()
    End Sub

    Public Sub Open()
        If _serial.IsOpen = True Then
            _serial.Close()
        End If
        Try
            _serial.Open()
        Catch ex As Exception
        End Try
    End Sub

    Public Sub FindRequest()
        Dim req = New Micronet.MicronetFrame
        req.Cmd = 255
        Request(req, 1000)
    End Sub

    Private Sub SendFrame(frame As MicronetFrame)
        Dim data(32 + frame.Data.Length) As Byte
        Array.Copy({CByte(&H55), CByte(&HAA), CByte(&H78), CByte(&H92)}, 0, data, 0, 4)
        Array.Copy(Tool.StringToByteArray(frame.SenderAddress), 0, data, 4, 12)
        Array.Copy(Tool.StringToByteArray(frame.TargetAddress), 0, data, 16, 12)
        data(28) = frame.Cmd
        Dim dlen As UInt16 = frame.Data.Length
        data(29) = dlen >> 8
        data(30) = dlen And 255
        Array.Copy(frame.Data, 0, data, 31, frame.Data.Length)
        Dim crc As UInt16 = GetChecksum(data, 4, 27 + frame.Data.Length)
        data(data.Length - 1) = crc And 255
        data(data.Length - 2) = crc >> 8
        _serial.Write(data, 0, data.Length)
    End Sub

    Public Function Request(frame As MicronetFrame, timeout As Integer) As MicronetFrame
        Dim _tempFrame As New MicronetFrame
        If _serial.IsOpen = False Then
                _tempFrame.State = MicronetFrame.FrameState.PortClosed
                Try
                    _serial.Open()
                Catch ex As Exception
                End Try
            End If
            SendFrame(frame)
        If _waitEvent.WaitOne(timeout) Then
            _tempFrame.Cmd = _currentFrame.Cmd
            _tempFrame.TargetAddress = _currentFrame.TargetAddress
            _tempFrame.SenderAddress = _currentFrame.SenderAddress
            _tempFrame.Data = _currentFrame.Data
            _tempFrame.State = _currentFrame.State
        Else
            _tempFrame.State = MicronetFrame.FrameState.Timeout
        End If
        Return _tempFrame
    End Function

    Private Sub DataReceivedHandler(sender As Object, e As SerialDataReceivedEventArgs)
        Dim serial As SerialPort = CType(sender, SerialPort)
        Try
            Dim data(serial.BytesToRead) As Byte
            Dim length = serial.BytesToRead
            serial.Read(_frameBytes, _frameBytesCursor, length)
            _frameBytesCursor += length
            FrameScanner()
        Catch ex As Exception
            serial.DiscardInBuffer()
            _frameBytesCursor = 0
        End Try

    End Sub

    Private Sub RequestAllDevicesInfo()
        Dim unknownList As New List(Of String)
        For Each device In Devices.Keys
            If Devices(device).Length = 0 Then
                unknownList.Add(device)
            End If
        Next
        For Each device In unknownList
            Dim frame = New MicronetFrame
            frame.Cmd = 254
            frame.TargetAddress = device
            Dim response = Request(frame, 1000)
            If response.State = MicronetFrame.FrameState.OK Then
                Dim deviceName = Text.Encoding.ASCII.GetString(response.Data)
                Devices(device) = deviceName
            End If
        Next
    End Sub

    Private Sub CreatFrameFromBytes()
        Dim dataLen = _frameBytes(29) * 256 + _frameBytes(30)
        _currentFrame.Cmd = _frameBytes(28)
        _currentFrame.SenderAddress = BitConverter.ToString(_frameBytes, 4, 12).Replace("-", "")
        _currentFrame.TargetAddress = BitConverter.ToString(_frameBytes, 16, 12).Replace("-", "")
        Dim crc = GetChecksum(_frameBytes, 4, 27 + dataLen)
        Dim data(dataLen - 1) As Byte
        _currentFrame.Data = data
        Array.Copy(_frameBytes, 31, _currentFrame.Data, 0, dataLen)
        Dim frameCrc = _frameBytes(31 + dataLen) * 256 + _frameBytes(32 + dataLen)
        If crc = frameCrc Then
            ShiftDataBuffer(33 + dataLen)
            _frameBytesCursor -= (33 + dataLen)
            _waitEvent.Set()
            If Devices.ContainsKey(_currentFrame.SenderAddress) = False Then
                Devices.Add(_currentFrame.SenderAddress, "")
            End If
            RaiseEvent OnFrameReceive(_currentFrame)
        End If
    End Sub

    Public Sub FrameScanner()
        Try
            Dim i = 0
            While i < _frameBytesCursor
                If _frameBytes(i) = &H55 AndAlso _frameBytes(i + 1) = &HAA AndAlso _frameBytes(i + 2) = &H78 AndAlso _frameBytes(i + 3) = &H92 Then
                    If i > 0 Then
                        ShiftDataBuffer(i)
                        _frameBytesCursor -= i
                    End If
                    CreatFrameFromBytes()
                    i = 0
                End If
                i += 1
            End While
        Catch ex As Exception
            Console.ForegroundColor = ConsoleColor.Red
            Console.WriteLine(ex.Message)
            Console.ForegroundColor = ConsoleColor.White
        End Try
    End Sub

    Private Function GetChecksum(ByRef data As Byte(), start As Integer, length As Integer) As UInt16
        Dim crc = &HFFFF
        For index = 0 To length - 1
            crc = crc Xor data(start + index)
            For i = 0 To 7
                If (crc And 1) > 0 Then
                    crc = (crc >> 1) Xor &HA001
                Else
                    crc = (crc >> 1)
                End If
            Next
        Next
        Return crc
    End Function

    Private Sub DeviceScanner()
        While True
            RequestAllDevicesInfo()
            Threading.Thread.Sleep(5000)
        End While
    End Sub
    Private Sub ShiftDataBuffer(index As Integer)
        Dim subArray(_frameBytes.Length - index - 1) As Byte
        Array.Copy(_frameBytes, index, subArray, 0, subArray.Length)
        Array.Copy(subArray, 0, _frameBytes, 0, subArray.Length)
    End Sub

    Public Class MicronetFrame
        Public SenderAddress As String = "FFFFFFFFFFFFFFFFFFFFFFFF"
        Public TargetAddress As String = "000000000000000000000000"
        Public Cmd As Byte = 0
        Public Data() As Byte = {}
        Public State As FrameState = FrameState.OK

        Public Enum FrameState As Byte
            Timeout
            OK
            PortClosed
        End Enum
    End Class
End Class
