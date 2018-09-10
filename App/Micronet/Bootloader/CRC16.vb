Public Class CRC16
    Private Shared _crcTable(255) As UInteger
    Sub New()
    End Sub

    Public Function GetChechSum(Data As Byte()) As UInteger
        Dim crc As UInt16 = &HFFFF
        For i = 0 To Data.Length - 1
            crc = crc Xor Data(i)
            For ii = 0 To 7
                If (crc And &H1) <> 0 Then
                    crc = (crc >> 1) Xor &HA001
                Else
                    crc = (crc >> 1)
                End If
            Next
        Next
        Return crc
    End Function
    '  int i;
    'micronet_crc ^= a;   
    'For (i = 0; i < 8; ++i){
    '    If (micronet_crc & 1) micronet_crc = (micronet_crc >> 1) ^ 0xA001;
    '    Else micronet_crc = (micronet_crc >> 1);
    '}
End Class
