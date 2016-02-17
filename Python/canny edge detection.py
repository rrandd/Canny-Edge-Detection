# Written by Ivan Reedman
# February 2016
# Canny Edge Detection implementation in Python

# import relevant extensions
from PIL import Image as image
from PIL import ImageDraw
from PIL import ImageFont
import os
import time
import math

# load source image
img_src=image.open("bridge.jpg")

# ensure image is 8bits per pixel
img_src=img_src.convert("L")

# get pixels
pixels_src=img_src.load()

# display image
img_src.show()

# get width and height of source image
width=img_src.size[0]
height=img_src.size[1]

#--------- gaussian blur pass --------
# create destination image for blur
img_blur=image.new("RGB",(width,height),"BLACK")
pixels_blur=img_blur.load()

# get current time
currenttime=time.time()

# display image basics
print "Image width=" + str(width) + " height=" + str(height)
print ""

# display current task
print "Applying gaussian blur to image"

# now calculate gradient intensity of each pixel
for x in range(2,width-2):
    for y in range(2,height-2):

        blurpixel=0

        # +-2 of current position then apply gaussian filter weightings
        for dx in range(-2,2):
            for dy in range(-2,2):
                pixelweight = 0
                if (abs(dy) == 2):
                    if (abs(dx) == 0):
                        pixelweight = 5
                    elif (abs(dx) == 1):
                        pixelweight = 4
                    elif (abs(dx) == 2):
                        pixelweight = 2
                elif (abs(dy) == 1):
                    if (abs(dx) == 0):
                        pixelweight = 12
                    elif (abs(dx) == 1):
                        pixelweight = 9
                    elif (abs(dx) == 2):
                        pixelweight = 4
                elif (abs(dy) == 0):
                    if (abs(dx) == 0):
                        pixelweight = 15
                    elif (abs(dx) == 1):
                        pixelweight = 12
                    elif (abs(dx) == 2):
                        pixelweight = 5

                # apply weighting
                blurpixel = blurpixel + pixels_src[x + dx, y + dy] * pixelweight
        
        # calculate actual result
        blurpixel = blurpixel / 159

        # store gradient into destination image
        pixels_blur[x,y]=(blurpixel,blurpixel,blurpixel)

# display time taken to process image
print "- Apply gaussian blur to image took " + str(round(time.time()-currenttime,3)) + " seconds"
print ""

# copy original to temp
img_temp=img_blur.copy()

# create canvas for drawing
draw = ImageDraw.Draw(img_temp)

# font = ImageFont.truetype(<font-file>, <font-size>)
font = ImageFont.truetype("arial.ttf", 16)

# draw.text((x, y),"message",(r,g,b))
draw.text((0, 0),"Gaussian Blur",(255,0,0),font=font)

# display gradient image and save
img_temp.show()
img_temp.save("blur.bmp","BMP")

# display current task
print "Gradient intensity pass"

#--------- gradient intensity and direction pass --------
# create destination image or black and white
img_grad=image.new("RGB",(width,height),"BLACK")
pixels_grad=img_grad.load()

# create image array for gradient direction
img_grad_dir=image.new("RGB",(width,height),"BLACK")
pixels_grad_dir=img_grad_dir.load()

# get current time
currenttime=time.time()

pix=[0,0,0]

# now calculate gradient intensity of each pixel
for x in range(1,width):
    for y in range(1,height):

        # get source pixels to calculate intensity and direction
        pix[0]=pixels_blur[x,y][0]      # main pixel
        pix[1]=pixels_blur[x-1,y][0]    # pixel left
        pix[2]=pixels_blur[x,y-1][0]    # pixel above

        # get value for x gradient
        grad_x=pix[0]-pix[1]

        # get value for y gradient
        grad_y=pix[0]-pix[2]

        # calculation gradient direction
        # we want this rounded to 0,1,2,3 which represent 0,45,90,135 degrees
        grad_dir = int(abs((math.atan2(grad_y,grad_x))+0.22))*80    # mul by 80 just so image has depth

        # store gradient direction
        pixels_grad_dir[x,y]=(grad_dir,grad_dir,grad_dir)

        # get absolute value for x gradient
        grad_x=abs(grad_x)
        
        # get absolute value for y gradient
        grad_y=abs(grad_y)
        
        # calculate gradient
        grad=int(math.sqrt(grad_x*grad_x+grad_y*grad_y)) * 2

        # store gradient into destination image
        pixels_grad[x,y]=(grad,grad,grad)

# display time taken to process image
print "- Calculating the intensity gradient of image took " + str(round(time.time()-currenttime,3)) + " seconds"
print ""

# copy original to temp
img_temp=img_grad.copy()

# create canvas for drawing
draw = ImageDraw.Draw(img_temp)

# draw.text((x, y),"message",(r,g,b))
draw.text((0, 0),"Gradient",(255,0,0),font=font)

# display gradient image and save
img_temp.show()
img_temp.save("gradient.bmp","BMP")

# copy original to temp
img_temp=img_grad_dir.copy()

# create canvas for drawing
draw = ImageDraw.Draw(img_temp)

# draw.text((x, y),"message",(r,g,b))
draw.text((0, 0),"Gradient Direction",(255,0,0),font=font)

# display gradient direction and save
img_temp.show()
img_temp.save("direction.bmp","BMP")

# display current task
print "Non-maximal suppression"

#--------- non-maximal suppression pass --------
# create destination image or black and white
img_nm=image.new("RGB",(width,height),"BLACK")
pixels_nm=img_nm.load()

# get current time
currenttime=time.time()

# set sensitivity
sensitivity=10

# now suppress non local maximal of each pixel based on direction
for x in range(1,width-1):
    for y in range(1,height-1):
        # is current pixel_grad above threshold
        if (pixels_grad[x,y][0]>=sensitivity):
            # based on direction
            grad_dir=pixels_grad_dir[x,y][0]    # divide by 80 as we multiplied it up just so image was visible

            # if 0 then vertical
            if (grad_dir==0):
                if (pixels_grad[x,y][0]>=pixels_grad[x,y-1][0] and pixels_grad[x,y][0]>= pixels_grad[x,y+1][0]):
                    # mark current pixel as maximal
                    pixels_nm[x,y]=(255,255,255)

                    # mark pixels either side based on direction as non-maximal
                    pixels_nm[x,y-1]=(0,0,0)
                    pixels_nm[x,y+1]=(0,0,0)
                else:
                    # mark current pixel as non-maximal
                    pixels_nm[x,y]=(0,0,0)
                
            # if 80 then 45 degrees
            elif (grad_dir==80):
                if (pixels_grad[x,y][0]>=pixels_grad[x+1,y-1][0] and pixels_grad[x,y][0]>= pixels_grad[x-1,y+1][0]):
                    # mark current pixel as maximal
                    pixels_nm[x,y]=(255,255,255)

                    # mark pixels either side based on direction as non-maximal
                    pixels_nm[x+1,y-1]=(0,0,0)
                    pixels_nm[x-1,y+1]=(0,0,0)
                else:
                    # mark current pixel as non-maximal
                    pixels_nm[x,y]=(0,0,0)

            # if 160 then 90 degrees
            elif (grad_dir==160):
                if (pixels_grad[x,y][0]>=pixels_grad[x-1,y][0] and pixels_grad[x,y][0]>= pixels_grad[x+1,y][0]):
                    # mark current pixel as maximal
                    pixels_nm[x,y]=(255,255,255)

                    # mark pixels either side based on direction as non-maximal
                    pixels_nm[x-1,y]=(0,0,0)
                    pixels_nm[x+1,y]=(0,0,0)
                else:
                    # mark current pixel as non-maximal
                    pixels_nm[x,y]=(0,0,0)

            # if 240 then 135 degrees
            elif (grad_dir==240):
                if (pixels_grad[x,y][0]>=pixels_grad[x-1,y-1][0] and pixels_grad[x,y][0]>= pixels_grad[x-1,y-1][0]):
                    # mark current pixel as maximal
                    pixels_nm[x,y]=(255,255,255)

                    # mark pixels either side based on direction as non-maximal
                    pixels_nm[x-1,y-1]=(0,0,0)
                    pixels_nm[x+1,y+1]=(0,0,0)
                else:
                    # mark current pixel as non-maximal
                    pixels_nm[x,y]=(0,0,0)
        else:
            # mark current pixel as non-maximal
            pixels_nm[x,y]=(0,0,0)
                
# display time taken to process image
print "- Calculating the non-maximal suppression pass of the image took " + str(round(time.time()-currenttime,3)) + " seconds"

# copy original to temp
img_temp=img_nm.copy()

# create canvas for drawing
draw = ImageDraw.Draw(img_temp)

# draw.text((x, y),"message",(r,g,b))
draw.text((0, 0),"Non-maximal suppression",(255,0,0),font=font)

# display nm image
img_temp.show()
img_temp.save("non-maximal.bmp","BMP")
