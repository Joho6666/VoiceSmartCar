# Hardware sources and EDA boundary

`VoiceSmartCar.kicad_sch` is an editable, text-annotated hardware connection review sheet. It presents intended subsystems and unknown interfaces without claiming to be a pin-level recreation of the photographed PCB. It contains no electrical symbols or asserted board nets; its exported netlist is intentionally empty. `PINOUT_TRACE.csv` and `../docs/HARDWARE_AUDIT.md` hold the pending measurement evidence.

No original CAD project, schematic, netlist, or Gerbers were present in the provided materials. The KiCad PDF is a connection review aid; it must not be used to wire the board without checking the measurement sheet. ERC's zero findings apply only to this text-only sheet; PCB connectivity, board DRC, circuit electrical rules and schematic-to-PCB cross-check are `UNAVAILABLE` until the physical measurements or original design data are supplied.

TI datasheets document the component pin functions; they do not establish the board's connection topology.
