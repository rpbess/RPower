Public Class MicronetFirmwareBootloader

    Public Property ApplicationStartAddress = 0
    Public Property ApplicationLength = 0

    Public Event OnProgress(target As String, percent As Integer)
    Public Event OnErrors(target As String, message As String)
    Public Event OnComplite(target As String)

    Private _hex As String = ""
    Private _targetAddress As String = ""
    Private _firmware As New Firmware

    Sub New()

    End Sub

    Public Sub SetTargetAddress(address As String)
        _targetAddress = address
    End Sub

    Public Sub DecodeHexText(text As String)
        _firmware.PrepareMemorySectors(text)
    End Sub

    Public Sub Upload(targetAddress As String, hexPath As String)
        _targetAddress = targetAddress
        _firmware.LoadFromHex(hexPath)
        Dim th As New Threading.Thread(AddressOf BootloaderProcess)
        th.IsBackground = True
        th.Start()
    End Sub

    Public Sub Upload()
        Dim th As New Threading.Thread(AddressOf BootloaderProcess)
        th.IsBackground = True
        th.Start()
    End Sub

    Private Sub EraseFlash()

    End Sub

    Private Sub SendHexToDevice(sector As Firmware.MemorySector)

    End Sub

    Public Sub RunApp()

    End Sub

    Public Sub RunBootloader()
    End Sub

    Private Sub BootloaderProcess()
        RaiseEvent OnProgress(_targetAddress, 0)
        Try
            Dim _crc As FrimwareCRC32 = New FrimwareCRC32(ApplicationStartAddress, ApplicationLength)
            RunBootloader()
            Threading.Thread.Sleep(1000)
            EraseFlash()
            Threading.Thread.Sleep(1000)
            Dim size = _firmware.GetFirmwareSize
            For i As Integer = 0 To size - 1
                Dim sector = _firmware.GetNext
                SendHexToDevice(sector)
                Threading.Thread.Sleep(100)
                If (Math.Round(i * 100 / size, 1)) > 96 Then
                    Console.WriteLine("")
                End If
                RaiseEvent OnProgress(_targetAddress, (Math.Round(i * 100 / size, 1)))
            Next
            RunApp()
            RaiseEvent OnComplite(_targetAddress)
        Catch ex As Exception
            RaiseEvent OnErrors(_targetAddress, ex.Message)
        End Try
    End Sub
End Class