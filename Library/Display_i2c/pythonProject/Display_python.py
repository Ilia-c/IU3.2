import pygame
import sys
import re

# -- Helper to parse C array data at runtime --------------------------------
def parse_c_array(filename, array_name):
    """
    Extracts a flat list of integers from a C array definition in the given file.
    Reads file using cp1251 encoding to support Cyrillic comments and identifiers.
    """
    pattern = rf"const\s+\w+\s+{array_name}\s*\[\]\s*=\s*\{{(.*?)\}};"
    with open(filename, 'r', encoding='cp1251', errors='ignore') as f:
        text = f.read()
    m = re.search(pattern, text, re.S)
    if not m:
        raise ValueError(f"Array {array_name} not found in {filename}")
    body = m.group(1)
    nums = re.findall(r"0x[0-9A-Fa-f]+|\d+", body)
    return [int(n, 0) for n in nums]

# -- Pixel font renderer ------------------------------------------------------
class PixelFont:
    def __init__(self, font_data):
        # Header: max_width, height, ascii_offset, declared_numchars (ignored)
        self.max_width, self.height, self.offset = font_data[:3]
        raw = font_data[4:]
        record_size = 1 + self.height
        self.numchars = len(raw) // record_size
        self.char_widths = []
        self.char_bitmaps = []
        for i in range(self.numchars):
            start = i * record_size
            w = raw[start]
            bmp = raw[start+1:start+1+w]
            self.char_widths.append(w)
            self.char_bitmaps.append(bmp)

    def render(self, surface, text, x, y, color=(0,0,0)):
        pos_x = x
        for ch in text:
            idx = ord(ch) - self.offset
            if idx < 0 or idx >= self.numchars:
                pos_x += self.max_width // 2
                continue
            w = self.char_widths[idx]
            bmp = self.char_bitmaps[idx]
            for cx in range(w):
                col = bmp[cx]
                for cy in range(self.height):
                    if (col >> cy) & 1:
                        surface.set_at((pos_x + cx, y + cy), color)
            pos_x += w + 1

# -- Menu classes -------------------------------------------------------------
class MenuItem:
    def __init__(self, label, suffix='', realtime=False):
        self.label = label     # string to display (ASCII)
        self.suffix = suffix
        self.realtime = realtime
        self.next = None
        self.prev = None
        self.child = None
        self.parent = None

class MenuEngine:
    def __init__(self, root, per_page=4):
        self.current = root
        self.index = 0
        self.per_page = per_page

    def move_up(self):
        if self.index > 0:
            self.index -= 1
        elif self.current.prev:
            self.current = self.current.prev
            self.index = 0

    def move_down(self):
        if self.index < self.per_page - 1:
            self.index += 1
        elif self.current.next:
            self.current = self.current.next
            self.index = 0

    def select(self):
        items = self.visible()
        if self.index < len(items) and items[self.index].child:
            self.current = items[self.index]
            self.index = 0

    def back(self):
        if self.current.parent:
            self.current = self.current.parent
            self.index = 0

    def visible(self):
        items = []
        node = self.current
        while node and len(items) < self.per_page:
            items.append(node)
            node = node.next
        return items

# -- Main function ------------------------------------------------------------
def main():
    pygame.init()
    scale = 4
    W, H = 128, 64
    screen = pygame.display.set_mode((W*scale, H*scale))
    clock = pygame.time.Clock()

    # Load the 5x7 pixel font
    font_data = parse_c_array('OLED_Fonst.c', 'my5x7fonts')
    font = PixelFont(font_data)

    # Define menu structure manually (analogous to MAKE_MENU)
    items = {}
    defs = [
        ('main',    'Modes'),
        ('settings','Settings'),
        ('status',  'Status'),
        ('network', 'Network'),
        ('info',    'Info'),
    ]
    for name, label in defs:
        items[name] = MenuItem(label)
    # Link next/prev
    items['main'].next = items['settings'];    items['settings'].prev = items['main']
    items['settings'].next = items['status'];  items['status'].prev = items['settings']
    items['status'].next = items['network'];   items['network'].prev = items['status']
    items['network'].next = items['info'];    items['info'].prev = items['network']
    # Set hierarchy: all under 'main'
    for name in ['settings','status','network','info']:
        items[name].parent = items['main']
    items['main'].child = items['settings']

    engine = MenuEngine(items['main'])

    while True:
        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                pygame.quit(); sys.exit()
            if ev.type == pygame.KEYDOWN:
                if ev.key == pygame.K_UP:     engine.move_up()
                elif ev.key == pygame.K_DOWN:  engine.move_down()
                elif ev.key == pygame.K_RETURN:engine.select()
                elif ev.key == pygame.K_BACKSPACE: engine.back()

        # Draw virtual OLED
        disp = pygame.Surface((W, H))
        disp.fill((255,255,255))  # white background
        for idx, itm in enumerate(engine.visible()):
            prefix = '>' if idx == engine.index else ' '
            text = prefix + itm.label + itm.suffix
            font.render(disp, text, 0, idx*(font.height+2))

        # Scale for visibility
        pygame.transform.scale(disp, (W*scale, H*scale), screen)
        pygame.display.flip()
        clock.tick(30)

if __name__ == '__main__':
    main()