-- Structural VAMS generated by gnetlist
-- Secondary unit
ARCHITECTURE SPICE_Diode_Model OF sp_Diode IS
    terminal unnamed_net2 :  electrical;
BEGIN
-- Architecture statement part
    CS1 : ENTITY CURRENT_SOURCE(voltage_dependend)
	GENERIC MAP (
			N => N, 
			VT => VT, 
			ISS => ISS)
	PORT MAP (	LT => unnamed_net2,
			RT => kathode);

    VD_CAP : ENTITY VOLTAGE_DEPENDEND_CAPACITOR
	GENERIC MAP (
			PB => PB, 
			M => M, 
			VT => VT, 
			ISS => ISS, 
			TT => TT, 
			CJ0 => CJ0)
	PORT MAP (	LT => kathode,
			RT => unnamed_net2);

    RES : ENTITY RESISTOR
	GENERIC MAP (
			r => RS)
	PORT MAP (	RT => anode,
			LT => unnamed_net2);

END ARCHITECTURE;