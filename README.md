# Lexia_J2534
Supported J2534 API Description:
- **PassThruOpen** - SUPPORTED
- **PassThruClose** - SUPPORTED
- **PassThruConnect** - Only ISO15765 with 500000 baud
- **PassThruDisconnect** - SUPPORTED
- **PassThruReadMsgs** - Only one ISO15765 message, timeout don't care
- **PassThruWriteMsgs** - Only one ISO15765 message, timeout don't care
- **PassThruStartPeriodicMsg** - NOT SUPPORTED
- **PassThruStopPeriodicMsg** - NOT SUPPORTED
- **PassThruStartMsgFilter** - Only FLOW_CONTROL_FILTER for ISO15765
- **PassThruStopMsgFilter** - SUPPORTED
- **PassThruSetProgrammingVoltage** - NOT SUPPORTED
- **PassThruReadVersion** - SUPPORTED
- **PassThruGetLastError** - TODO
- **PassThruIoctl** - Supports only READ_VBATT (const value 14.4v), CLEAR_RX_BUFFER, CLEAR_TX_BUFFER - only tx buffer present, CLEAR_MSG_FILTERS, other - not supported
