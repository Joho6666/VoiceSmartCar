"""Print one protocol byte, or explicitly send it to an explicitly named UART."""
import argparse

COMMANDS = {
    "A1": (0xA1, "forward"),
    "A2": (0xA2, "reverse"),
    "A3": (0xA3, "stop"),
    "A4": (0xA4, "faster"),
    "A5": (0xA5, "slower"),
    "A6": (0xA6, "left"),
    "A7": (0xA7, "right"),
    "B1": (0xB1, "sequence 1"),
    "B2": (0xB2, "sequence 2"),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=COMMANDS, help="single-byte HEX command")
    parser.add_argument("--port", help="explicit COM port; omitted means dry-run only")
    args = parser.parse_args()
    value, label = COMMANDS[args.command]
    if args.port is None:
        print(f"DRY RUN: {args.command} -> 0x{value:02X} ({label}); no COM port opened")
        return 0
    try:
        import serial
    except ImportError:
        parser.error("pyserial is required for --port; install it with: py -m pip install pyserial")
    with serial.Serial(args.port, 9600, bytesize=8, parity="N", stopbits=1, timeout=0.5) as port:
        port.write(bytes((value,)))
        port.flush()
    print(f"SENT: 0x{value:02X} ({label}) to {args.port} at 9600 8N1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
