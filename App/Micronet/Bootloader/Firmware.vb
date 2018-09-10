
Public Class Firmware
    Private _packets As List(Of MemorySector) = New List(Of MemorySector)

    Sub New()
    End Sub

    Public Function GetFirmwareSize() As UInteger
        Return _packets.Count
    End Function

    Public Function GetNext() As MemorySector
        If _packets.Count > 0 Then
            Dim d = _packets.First
            _packets.RemoveAt(0)
            Return d
        End If
        Return Nothing
    End Function

    Private Function hexLineDecode(hex As String) As HexLineData
        Dim _decoded = New HexLineData
        Dim dataLength As UInt16 = Convert.ToUInt16(hex.Substring(0, 2), 16)
        _decoded.HexAddress = Convert.ToUInt16(hex.Substring(2, 4), 16)
        _decoded.HexType = Convert.ToUInt16(hex.Substring(6, 2), 16)
        If _decoded.HexType = 0 Then
            _decoded.HexData = Tool.DecodeHexData(hex.Substring(8, dataLength * 2))
        End If
        If _decoded.HexType = 4 Then
            Dim temp = Tool.StringToByteArray(hex.Substring(8, 4))
            _decoded.HexAddress = temp(0) * 256 + temp(1)
        End If
        Return _decoded
    End Function

    Public Sub LoadFromHex(path As String)
        PrepareMemorySectors(IO.File.ReadAllText(path))
    End Sub

    Public Sub PrepareMemorySectors(hexText As String)
        _packets.Clear()
        Dim lines = hexText.Split(":")
        Console.WriteLine("HEX decoder -> Received hex lines: " + lines.Length.ToString)
        Dim offset As UInt32 = 0
        Dim i As UInt16 = 0
        While i < lines.Length - 1
            If (lines(i).Length > 8) Then
                Dim _sector = New MemorySector
                Dim _sectorAddressCounter As UInt32 = 0
                While _sector.FlashData.Count < 150
                    Dim oneLine = hexLineDecode(lines(i))
                    If oneLine.HexType = 4 Then
                        i = i + 1
                        offset = oneLine.HexAddress
                        Exit While
                    End If
                    If oneLine.HexType = 0 Then
                        If _sector.FlashAddress = 0 Then
                            _sector.FlashAddress = (offset << 16) Or oneLine.HexAddress
                            _sectorAddressCounter = oneLine.HexAddress
                        End If
                        If _sectorAddressCounter = oneLine.HexAddress Then
                            _sector.AppendData(oneLine.HexData)
                            _sectorAddressCounter += oneLine.HexData.Length
                            i = i + 1
                        Else
                            Exit While
                        End If
                    End If
                    If oneLine.HexType <> 0 And oneLine.HexType <> 4 Then
                        i = i + 1
                        Exit While
                    End If
                End While
                If _sector.FlashData.Count > 0 Then
                    _packets.Add(_sector)
                End If
            Else
                i = i + 1
            End If
        End While
    End Sub

    Private Class HexLineData
        Public HexType As UInt16 = 0
        Public HexAddress As UInt16 = 0
        Public HexData As Byte()
    End Class

    Public Class MemorySector
        Public Property FlashAddress As UInt32 = 0
        Public Property FlashData As List(Of Byte) = New List(Of Byte)

        Sub New()
        End Sub

        Public Sub AppendData(data As Byte())
            FlashData.AddRange(data)
        End Sub

        Public Function PacketSerialization() As Byte()
            Dim list As List(Of Byte) = New List(Of Byte)
            list.Add(CByte(FlashData.Count))
            list.Add(CByte((FlashAddress >> 24) And &HFF))
            list.Add(CByte((FlashAddress >> 16) And &HFF))
            list.Add(CByte((FlashAddress >> 8) And &HFF))
            list.Add(CByte(FlashAddress And &HFF))
            list.AddRange(FlashData)
            Dim _crc = New FrimwareCRC32(0, FlashData.Count)
            _crc.AppendFirmwareData(0, FlashData.ToArray)
            Dim crcValue = _crc.GetCRC32
            list.Add(CByte((crcValue >> 24) And &HFF))
            list.Add(CByte((crcValue >> 16) And &HFF))
            list.Add(CByte((crcValue >> 8) And &HFF))
            list.Add(CByte(crcValue And &HFF))
            Return list.ToArray
        End Function
    End Class
End Class
