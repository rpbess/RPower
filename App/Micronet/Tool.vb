Public Class Tool
    Shared table As UInteger()
    Shared Sub New()
        Dim poly As UInteger = &HEDB88320UI
        table = New UInteger(255) {}
        Dim temp As UInteger = 0
        For i As UInteger = 0 To table.Length - 1
            temp = i
            For j As Integer = 8 To 1 Step -1
                If (temp And 1) = 1 Then
                    temp = CUInt((temp >> 1) Xor poly)
                Else
                    temp >>= 1
                End If
            Next
            table(i) = temp
        Next
    End Sub

    Public Function ComputeChecksum(bytes As Byte()) As UInteger
        Dim crc As UInteger = &HFFFFFFFFUI
        For i As Integer = 0 To bytes.Length - 1
            Dim index As Byte = CByte(((crc) And &HFF) Xor bytes(i))
            crc = CUInt((crc >> 8) Xor table(index))
        Next
        Return Not crc
    End Function

    Public Shared Function ByteArrayToString(ByVal ba As Byte()) As String
        Dim hex As String = BitConverter.ToString(ba)
        Return hex.Replace("-", "")
    End Function

    Public Shared Function StringToByteArray(ByVal hex As String) As Byte()
        Dim NumberChars As Integer = hex.Length
        Dim bytes As Byte() = New Byte(NumberChars / 2 - 1) {}
        For i As Integer = 0 To NumberChars - 1 Step 2
            bytes(i / 2) = Convert.ToByte(hex.Substring(i, 2), 16)
        Next
        Return bytes
    End Function

    Public Shared Function GenerateRandomString(ByRef len As Integer) As String
        Using hasher As Security.Cryptography.MD5 = Security.Cryptography.MD5.Create()
            Dim dbytes As Byte() = hasher.ComputeHash(Text.Encoding.UTF8.GetBytes(CLng((DateTime.UtcNow - New DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc)).TotalMilliseconds).ToString))
            Dim sBuilder As New Text.StringBuilder()
            For n As Integer = 0 To dbytes.Length - 1
                sBuilder.Append(dbytes(n).ToString("X2"))
            Next n
            Dim str = sBuilder.ToString()
            If str.Length > len Then
                str = str.Substring(0, len)
            End If
            Return str
        End Using
    End Function

    ''' <summary>
    ''' Переводим строку HEX в массив байт
    ''' </summary>
    ''' <param name="hex">Строка с HEX данными</param>
    ''' <returns></returns>
    Public Shared Function DecodeHexData(hex As String) As Byte()
        Dim data(hex.Length / 2 - 1) As Byte
        For i = 0 To data.Length - 1
            data(i) = Convert.ToInt16(hex.Substring(i * 2, 2), 16)
        Next
        Return data
    End Function

End Class