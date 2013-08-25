import glob
import Image
import os
import sys

path = os.path


FORMAT_BUFFER = '''
const uint8_t %s[%d][%d] = {
   %s
};
'''


vars_declared = [];


def imagefile_to_c(filename):
   image = Image.open(filename)

   varname = path.splitext(path.basename(filename))[0]
   varname = varname.replace(' ', '');
   varname = varname.replace('-', '');
   varname = 'img_' + varname;
   vars_declared.append(varname);

   (cx, cy) = image.size
   rows = []

   for y in range(0, cy):
      row = []
      for x in range(0, cx):
         pixel = image.getpixel((x, y))
         if len(pixel) == 4: # flatten alpha channel
            alpha = pixel[3]
            pixel = [ comp * alpha / 255 for comp in pixel[:3]]
         row.extend(pixel)
      rows.append('{ ' + ','.join([str(byte) for byte in row]) + ' }')

   rows = ',\n   '.join(rows);

   print FORMAT_BUFFER % (varname, cy, cx * 3, rows)


for filespec in sys.argv[1:]:
   files = glob.glob(filespec)
   for filename in files:
      imagefile_to_c(filename)


print '''
const ImageInfo these_images[] = {
   %s
};
''' % (',\n   ').join([ 'IMAGE(%s)' % varname for varname in vars_declared])
