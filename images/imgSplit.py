import Image
import os
import sys



filename = sys.argv[1]
(keepX, keepY, skipX, skipY, resizeX, resizeY) = [int(x) for x in sys.argv[2:]]
fileComps = os.path.splitext(filename)


image = Image.open(filename)
(width, height) = image.size

numX = width / (keepX + skipX)
numY = height / (keepY + skipY)

for ix in range(numX):
   x = ix * (keepX + skipX) + skipX
   for iy in range(numY):
      y = iy * (keepY + skipY) + skipY

      crop = image.crop((x, y, x + keepX, y + keepY))
      crop = crop.resize((resizeX, resizeY))
      outfile = "%s_%d_%d%s" % (fileComps[0], ix, iy, fileComps[1])
      crop.save(outfile)
