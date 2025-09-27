#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A script to calculate the decimal value and x86 vftable index 
from a hexadecimal offset.

The vftable index is calculated by dividing the decimal offset by the pointer
size, which is 4 bytes for the x86 (32-bit) architecture.
"""

import sys

# For x86 (32-bit), a pointer is 4 bytes.
X86_POINTER_SIZE = 4

def calculate_and_print(hex_string):
    """
    Converts a hexadecimal string to its decimal value and calculates the
    corresponding x86 vftable index, then prints the results.

    Args:
        hex_string (str): The hexadecimal value (e.g., "1c", "0x1c").
    """
    try:
        # Allow inputs with or without the '0x' prefix
        decimal_value = int(hex_string, 16)
    except ValueError:
        print(f"\n[Error] '{hex_string}' is not a valid hexadecimal value. Please try again.")
        return

    # The vftable index is the decimal offset divided by the pointer size.
    index = decimal_value // X86_POINTER_SIZE
    remainder = decimal_value % X86_POINTER_SIZE

    # --- Output the results ---
    print("\n" + "="*35)
    print("--- Calculation Result ---")
    print(f"      Input Hex: 0x{decimal_value:x}")
    print(f"  Decimal Value: {decimal_value}")
    print("---------------------------------")
    print(f"x86 vftable Index: {index}   ({decimal_value} / {X86_POINTER_SIZE})")

    # Add a warning if the offset is not a multiple of the pointer size,
    # as it likely doesn't point to the start of a vtable entry.
    if remainder != 0:
        print("\n" + "!"*40)
        print("  WARNING: The offset is not a multiple of 4.\n"
              "  This may not point to the start of a valid\n"
              "  function pointer in the vftable.")
        print("!"*40)
    
    print("="*35)

def main():
    """Main entry point of the script."""
    print("--- Interactive x86 vftable Index Calculator ---")
    print("Enter a hexadecimal offset (e.g., '1c' or '0x48').")
    
    try:
        while True:
            # Prompt the user for input
            user_input = input("\nEnter hex offset (or 'quit' to exit): ").strip()

            # Check for exit conditions
            if user_input.lower() in ('quit', 'exit', 'q', ''):
                print("Exiting.")
                break
            
            # Perform the calculation and print the results
            calculate_and_print(user_input)

    except KeyboardInterrupt:
        print("\n\nScript aborted by user. Exiting.")
        sys.exit(0)

if __name__ == "__main__":
    main()