#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A script to solve for an unknown address using the ASLR rebasing formula:
  Runtime Target = Runtime Base + (Static Target - Static Base)

The script will prompt for each value. Enter the three known hex values
and leave the unknown value blank by pressing Enter.
"""

import sys

def parse_hex_input(hex_string):
    """
    Parses a hexadecimal string from user input.
    - Returns None if the string is empty (user pressed Enter), '?', or 'x'.
    - Returns an integer if the string is a valid hex value.
    - Exits with an error for invalid input.
    """
    # Treat empty string, '?', or 'x' as the unknown value
    if hex_string.strip().lower() in ('', '?', 'x'):
        return None
    
    try:
        # The int() constructor with base 16 handles '0x' prefixes automatically
        return int(hex_string, 16)
    except ValueError:
        print(f"\n[Error] '{hex_string}' is not a valid hexadecimal value.", file=sys.stderr)
        print("Please restart the script and try again.", file=sys.stderr)
        sys.exit(1)

def get_addresses():
    """
    Prompts the user to enter the four addresses.
    """
    print("Please enter the three known hexadecimal addresses.")
    print("For the value you want to calculate, just press Enter.\n")

    prompts = {
        'rta': "Enter Runtime Target Address (RTA): ",
        'rba': "Enter Runtime Base Address (RBA):  ",
        'sta': "Enter Static Target Address (STA):  ",
        'sba': "Enter Static Base Address (SBA):  "
    }
    
    addresses = {}
    for key, prompt_text in prompts.items():
        user_input = input(prompt_text)
        addresses[key] = parse_hex_input(user_input)
        
    # Validate that exactly one value was left blank
    unknowns = [key for key, val in addresses.items() if val is None]
    if len(unknowns) != 1:
        print(f"\n[Error] Expected exactly one unknown value, but found {len(unknowns)}.", file=sys.stderr)
        print("Please provide three values and leave one blank.", file=sys.stderr)
        sys.exit(1)
        
    return addresses

def solve_and_print(addr):
    """
    Solves for the unknown address and prints the results.
    """
    rta, rba, sta, sba = addr['rta'], addr['rba'], addr['sta'], addr['sba']
    unknown_name = ""

    # Rearrange the formula to solve for the unknown variable
    if rta is None:
        calculated_value = rba + (sta - sba)
        unknown_name = "Runtime Target Address (RTA)"
        rta = calculated_value
    elif rba is None:
        calculated_value = rta - sta + sba
        unknown_name = "Runtime Base Address (RBA)"
        rba = calculated_value
    elif sta is None:
        calculated_value = rta - rba + sba
        unknown_name = "Static Target Address (STA)"
        sta = calculated_value
    elif sba is None:
        calculated_value = sta - rta + rba
        unknown_name = "Static Base Address (SBA)"
        sba = calculated_value

    print("\n" + "="*40)
    print("--- Calculation Result ---")
    print(f"Calculated {unknown_name}:\n  >>> {hex(calculated_value)} <<<")
    print("\n--- Full Equation ---")
    print(f"RTA ({hex(rta)}) = RBA ({hex(rba)}) + \n       (STA ({hex(sta)}) - SBA ({hex(sba)}))")
    
    # Verification
    if rta == rba + (sta - sba):
        print("\nVerification: Success!")
    else:
        print("\nVerification: Failed! Check your inputs.", file=sys.stderr)
    print("="*40)

def main():
    """Main entry point of the script."""
    print("--- Memory Rebase Calculator ---")
    print("Solves: RTA = RBA + (STA - SBA)")
    
    try:
        addresses = get_addresses()
        solve_and_print(addresses)
    except KeyboardInterrupt:
        print("\n\nScript aborted by user. Exiting.")
        sys.exit(0)

if __name__ == "__main__":
    main()