import os, json

icons = json.load(open('icons.json'))

out = open('../src/imgui/IconsMaterialCommunity.h', 'w')

out.write('''
// Generated from https://github.com/Templarian/MaterialDesign/blob/master/meta.json
// for use with the font from https://materialdesignicons.com/
#pragma once

#define FONT_ICON_FILE_NAME_MC "materialdesignicons-webfont.ttf"

'''.strip() + '\n\n')

min_codepoint = 0xFFFFF
max_codepoint = 0
max_16_code = 0

string = ''

for icon in icons:
    # icon-name -> ICON_NAME
    name = icon['name'].replace('-', '_').upper()
    
    code = int(icon['codepoint'], 16)
    min_codepoint = min(min_codepoint, code)
    max_codepoint = max(max_codepoint, code)
    
    if code > max_16_code and code <= 0xFFFF:
        max_16_code = code
    
    # trim off b' and '
    codepoint = str(chr(code).encode('utf-8'))[2:-1]
    string += f'#define ICON_MC_{name} "{codepoint}" // U+{icon["codepoint"]}\n'

out.write(f'#define ICON_MIN_MC {"0x{:04x}".format(min_codepoint)}\n')
out.write(f'#define ICON_MAX_16_MC {"0x{:04x}".format(max_16_code)}\n')
out.write(f'#define ICON_MAX_MC {"0x{:04x}".format(max_codepoint)}\n')
out.write(string)