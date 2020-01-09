# -*- coding: utf-8 -*-

from parsers import blender

class blender_cycles(blender.blender):
    """Blender Cycles
    """

    def __init__(self):
        blender.blender.__init__(self)

    def extractPercentage(self, line, left_token, right_token):
        """Tries to extract the percentage of the current render progress in this line.
        
        Works with the old syntax:
            (...) RenderLayer | Rendered 136/510 Tiles, Denoised 96 tiles
        And the new syntax:
            (...) RenderLayer | Path Tracing Tile  136/510, Denoised 96 tiles
        
        Args:
            line (string) : blender cycles render output line to parse
            left_token (string) : separator string to extract the percentage on the left side
            right_token (string) : separator string to extract the percentage on the right side
            
        Returns:
            (bool) : True if a percentage got extracted, False if not
            (int)  : percentage value
            
        """
        tokens = line.split(left_token)
        if len(tokens) > 1:
            tokens = tokens[1].split(right_token)   
            numbers = tokens[0].split('/')
            if len(numbers) == 2:
                try:
                    part0 = int(numbers[0])
                    part1 = int(numbers[1])
                    if part1 > 0:
                        perc = int(100 * part0 / part1)
                        return True, perc
                except:
                    pass
        return False, 0

    
    def do(self, data, mode):
        lines = data.split('\n')
        need_calc = False
        for line in lines:
            # deal with new syntax
            status, perc = self.extractPercentage(line, '| Rendered ', 'Tiles')
            if status:
                self.percentframe = perc
                need_calc = True
            # deal with old syntax
            status, perc = self.extractPercentage(line, '| Path Tracing Tile ', ',')
            if status:
                self.percentframe = perc
                need_calc = True
                        
        if need_calc:
            self.calculate()
        blender.blender.do(self, data, mode)
