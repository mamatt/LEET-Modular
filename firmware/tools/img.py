
import numpy as np
import cv2                      # pip install opencv-python
import matplotlib.pyplot as plt  # used to show image
import os                       # used for filesize


def show(img, figsize=(10, 10), title="Image"):
    figure = plt.figure(figsize=figsize)
    plt.imshow(img)
    plt.show()


def monochrome(img):
    encoded = []
    th = 127
    for pixel in img:
        if pixel < th:
            encoded.append(0)
        else:
            encoded.append(1)
    return np.array(encoded)


def xorRowCompress(img, col):
    for x in range(1, len(img)-col):
        img[len(img)-x] ^= img[len(img)-x-col]
    return np.array(img)


def RLE_encoding(img):
    encoded = []
    count = -1
    prev = 0
    for pixel in img:
        if prev != pixel:
            encoded.append(count)
            prev = pixel
            count = -1
        if count == 254:
            encoded.append(255)
            count = -1
        count += 1
    encoded.append(count)
    return np.array(encoded)


def combo_decode(encoded):
    decoded = []
    preRow = [0] * 240
    pixel = 0
    invert = 0
    x = 0
    for pixelCount in encoded:
        if pixelCount < 255:
            pixelCount += 1
            invert = 1
        for z in range(pixelCount):
            preRow[x] ^= pixel
            decoded.append(preRow[x])
            x += 1
            if x == 240:
                x = 0
        if invert == 1:
            pixel ^= 1
            invert = 0
    return np.array(decoded)


fpath = "in.png"
img = cv2.imread(fpath, 0)
shape = img.shape
fimg = img.flatten()
fimg = monochrome(fimg)
ximg = fimg     # copy for file size comparison
# show(fimg.reshape(shape))
xorRowCompress(ximg, 240)
encoded = RLE_encoding(ximg)    # <== encoded contains the compressed image

# decode and show to verify result
oimg = combo_decode(encoded)
show(oimg.reshape(shape))

# save with different file formats for file size comparison
cv2.imwrite("out.tif", fimg.reshape(shape))
cv2.imwrite("out.png", fimg.reshape(shape))
cv2.imwrite("out.jpg", fimg.reshape(shape))
f = open("out.jvk", "wb")
for x in encoded.astype(np.uint8):
    f.write(x)
f.close()

# print size of each file
print("16bit size:" + str(len(fimg)*2))
print("mono size:" + str(len(fimg)/8))
files = ["out.tif", "out.jpg", "out.png", "out.jvk"]
for f in files:
    print(f"File: {f} => Size: {os.path.getsize(f)} Bytes")
print("encoded bytes: "+str(len(encoded)))

print("\nconst uint8_t img["+str(len(encoded))+"] = {" + ','.join(str(x)
      for x in encoded.astype(np.uint8)) + "};")
