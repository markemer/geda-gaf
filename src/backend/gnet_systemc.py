# xorn.geda.netlist - gEDA Netlist Extraction and Generation
# Copyright (C) 1998-2010 Ales Hvezda
# Copyright (C) 1998-2010 gEDA Contributors (see ChangeLog for details)
# Copyright (C) 2013-2017 Roland Lutz
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# SystemC netlist backend written by Jaume Masip
# (based on gnet-verilog.scm by Mike Jarabek)

import re

# some useful regexes for working with net names

id_regexp = '[a-zA-Z_][a-zA-Z_\d$]*'

# match on a systemc identifier like: netname[x:y]
bit_range_reg = re.compile('^(' + id_regexp + ')\s*'
                           '\[\s*(\d+)\s*:\s*(\d+)\s*\]')

# match on a systemc identifier like: netname[x]
single_bit_reg = re.compile('^(' + id_regexp + ')\s*'
                            '\[\s*(\d+)\s*\]')

# match on a systemc identifier like: netname<type>
systemc_reg = re.compile('^(' + id_regexp + ')\s*'
                         '<\s*(' + id_regexp + ')\s*>')

# match on a systemc identifier like: netname
simple_id_reg = re.compile('^(' + id_regexp + ')$')

# return a list of nets whose pins have the desired attribute
# name/value pair

def get_matching_nets(netlist, attribute, value):
    return [package.pins[-1].net
            for package in reversed(netlist.packages)
            if package.get_attribute(attribute, 'unknown') == value]

# Take a netname and return the name of the wire.

def get_netname(net):
    # check over each expression type, and build the appropriate result

    # is it a bit range?
    match = bit_range_reg.match(net.name)
    if match is not None:
        return match.group(1)

    # just a single bit?
    match = single_bit_reg.match(net.name)
    if match is not None:
        return match.group(1)

    # just a systemc signal?
    match = systemc_reg.match(net.name)
    if match is not None:
        return match.group(1)

    # or a net without anything
    match = simple_id_reg.match(net.name)
    if match is not None:
        return match.group(1)

    net.warn("not likely a valid Verilog identifier")
    return net.name

# Highest level function
# Write Structural systemc representation of the schematic

def run(f, netlist):
    # write top level header
    f.write('/* structural SystemC generated by gnetlist */\n')
    f.write('/* WARNING: This is a generated file, edits */\n')
    f.write('/*        made here will be lost next time  */\n')
    f.write('/*        you run gnetlist!                 */\n')
    f.write('/* Id ........gnet-systemc.scm (04/09/2003) */\n')
    f.write('/* Source...../home/geda/gnet-systemc.scm   */\n')
    f.write('/* Revision...0.3 (23/09/2003)              */\n')
    f.write('/* Author.....Jaume Masip                   */\n')
    f.write('\n')

    # return the top level block name for the module
    module_name = netlist.get_toplevel_attribute('module_name', 'not found')

    # build port lists
    #
    # Scan through the list of components, and pins from each one,
    # finding the pins that have PINTYPE == CHIPIN, CHIPOUT, CHIPTRI
    # (for inout).  Build three lists, one each for the inputs,
    # outputs and inouts.

    in_ports = get_matching_nets(netlist, 'device', 'IPAD')
    out_ports = get_matching_nets(netlist, 'device', 'OPAD')
    inout_ports = get_matching_nets(netlist, 'device', 'IOPAD')

    # write module declaration
    f.write('#include "systemc.h"\n')
    for package in reversed(netlist.packages):
        device = package.get_attribute('device', 'unknown')
        # ignore specials
        if device not in ['IOPAD', 'IPAD', 'OPAD', 'HIGH', 'LOW']:
            f.write('#include "%s.h"\n' % device)
    f.write('\n')

    f.write('SC_MODULE (%s)\n' % module_name)
    f.write('{\n')
    f.write('\n')

    # output the module direction section
    f.write('/* Port directions begin here */\n')
    # do each input, output, and inout
    for net in in_ports:
        f.write('sc_in<bool> %s;\n' % get_netname(net))
    for net in out_ports:
        f.write('sc_out<bool> %s;\n' % get_netname(net))
    for net in inout_ports:
        f.write('sc_inout<bool> %s;\n' % get_netname(net))
    f.write('\n')

    # Loop over the list of nets in the design, writing one by one
    f.write('/* Wires from the design */\n')
    # print a wire statement for each
    done_nets = set()
    for net in reversed(netlist.nets):
        parsed_netname = get_netname(net)
        if parsed_netname not in done_nets:
            f.write('sc_signal<0> %s;\n' % parsed_netname)
            done_nets.add(parsed_netname)
    f.write('\n')

    # Output any continuous assignment statements generated
    # by placing `high' and `low' components on the board
    # TODO: fixme, multiple bit widths!
    for wire in get_matching_nets(netlist, 'device', 'HIGH'):
        f.write("assign %s = 1'b1;\n" % wire.name)
    for wire in get_matching_nets(netlist, 'device', 'LOW'):
        f.write("assign %s = 1'b0;\n" % wire.name)
    f.write('\n')

    # Output a compoment instatantiation for each of the
    # components on the board

    f.write('/* Package instantiations */\n')

    for package in reversed(netlist.packages):
        device = package.get_attribute('device', 'unknown')
        # ignore specials
        if device not in ['IOPAD', 'IPAD', 'OPAD', 'HIGH', 'LOW']:
            f.write('%s %s;\n' % (device, package.refdes))
    f.write('\n')

    f.write('SC_CTOR(%s):\n' % module_name)
    c_p = False
    for package in reversed(netlist.packages):
        device = package.get_attribute('device', None)
        # ignore specials
        if device in ['IOPAD', 'IPAD', 'OPAD', 'HIGH', 'LOW']:
            continue

        if c_p:
            f.write(',\n')
        else:
            c_p = True
        f.write('    %s("%s' % (package.refdes, package.refdes))
        for lp in xrange(32):
            attr = 'attr%d' % (lp + 1)
            description = package.get_attribute(attr, None)
            if description is not None:
                f.write('","')
                f.write(description)
        f.write('")')
    f.write('\n')

    f.write('  {')
    for package in reversed(netlist.packages):
        device = package.get_attribute('device', None)
        # ignore specials
        if device in ['IOPAD', 'IPAD', 'OPAD', 'HIGH', 'LOW']:
            continue

        # output a module connection for the package with named ports
        if package.pins:
            f.write('\n')
        for pin in package.pins:
            if pin.net.is_unconnected_pin:
                continue

            # Display the individual net connections
            match = systemc_reg.match(pin.net.name)
            if match is not None:
                net_str = match.group(1)
            else:
                net_str = pin.net.name
            f.write('    %s.%s(%s);\n' % (package.refdes, pin.number, net_str))
    f.write('  }\n')

    f.write('};\n')
    f.write('\n')