Public Class FrimwareCRC32
    Private Shared CrcTable(255) As UInteger
    Dim crc As UInteger = &HFFFFFFFFUI
    Private _currentAddress As UInt32 = 0
    Private _endAddress As UInt32 = 0

    Sub New(startAdress As UInt32, applicationLength As UInt32)
        _currentAddress = startAdress
        _endAddress = startAdress + applicationLength
        Dim poly As UInteger = &HEDB88320UI
        CrcTable = New UInteger(255) {}
        Dim temp As UInteger = 0
        For i As UInteger = 0 To CrcTable.Length - 1
            temp = i
            For j As Integer = 8 To 1 Step -1
                If (temp And 1) = 1 Then
                    temp = CUInt((temp >> 1) Xor poly)
                Else
                    temp >>= 1
                End If
            Next
            CrcTable(i) = temp
        Next
    End Sub

    Public Sub AppendFirmwareData(address As UInt32, dataBytes As Byte())
        While _currentAddress < address
            _currentAddress = _currentAddress + 1
            AppendData(&HFF)
        End While
        For Each dataByte In dataBytes
            _currentAddress = _currentAddress + 1
            AppendData(dataByte)
        Next
    End Sub

    Private Sub AppendData(dataByte As UInt32)
        crc = CUInt((crc >> 8) Xor CrcTable(((crc) And &HFF) Xor dataByte))
    End Sub

    Public Function GetCRC32() As UInt32
        While _currentAddress < _endAddress - 4 Or (Not crc <> &HC771EDF2)
            _currentAddress = _currentAddress + 1
            AppendData(&HFF)
        End While
        Return Not CRC
    End Function
End Class