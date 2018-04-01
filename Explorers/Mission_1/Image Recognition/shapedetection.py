import math
import numpy as np
import cv2
import argparse
import imutils
from colorlabeler import ColorLabeler

lowerBound=np.array([136,87,111])
upperBound=np.array([180,255,255])

kernelOpen=np.ones((5,5))
kernelClose=np.ones((20,20))

contours = {}
approx = []

scale = 1

#camera
cap = cv2.VideoCapture(0)
cl = ColorLabeler()



#calculate angle
def angle(pt1,pt2,pt0):
    dx1 = pt1[0][0] - pt0[0][0]
    dy1 = pt1[0][1] - pt0[0][1]
    dx2 = pt2[0][0] - pt0[0][0]
    dy2 = pt2[0][1] - pt0[0][1]
    return float((dx1*dx2 + dy1*dy2))/math.sqrt(float((dx1*dx1 + dy1*dy1))*(dx2*dx2 + dy2*dy2) + 1e-10)




while(cap.isOpened()):
    
    ret, frame = cap.read()
    _, frame2 = cap.read()
    _, image = cap.read()

    
    resized = imutils.resize(image, width=700)
    ratio = image.shape[0] / float(resized.shape[0])
    blurred = cv2.GaussianBlur(resized, (5, 5), 0)
    gray = cv2.cvtColor(blurred, cv2.COLOR_BGR2GRAY)
    lab = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

 


    imgHSV= cv2.cvtColor(frame2,cv2.COLOR_BGR2HSV)
    # create the Mask
    mask=cv2.inRange(imgHSV,lowerBound,upperBound)
    #morphology
    maskOpen=cv2.morphologyEx(mask,cv2.MORPH_OPEN,kernelOpen)
    maskClose=cv2.morphologyEx(maskOpen,cv2.MORPH_CLOSE,kernelClose)

    maskFinal=maskClose
    _,conts,h=cv2.findContours(maskFinal.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_NONE)
    
    cv2.drawContours(frame2,conts,-1,(255,0,0),3)

    #color detection
    

    if ret==True:
        #grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        #Canny
        canny = cv2.Canny(frame,80,240,3)

        #contours
        canny2, contours, hierarchy = cv2.findContours(canny,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
        for i in range(0,len(contours)):
            
            approx = cv2.approxPolyDP(contours[i],cv2.arcLength(contours[i],True)*0.02,True)

            #Skip small or non-convex objects
            if(abs(cv2.contourArea(contours[i]))<100):
                continue

            #triangle
            if(len(approx) == 3):
                x,y,w,h = cv2.boundingRect(contours[i])
                cv2.putText(resized,'TRI',(x,y),cv2.FONT_HERSHEY_SIMPLEX,scale,(255,255,255),2,cv2.LINE_AA)
                cv2.rectangle(resized,(x,y),(x+w,y+h),(0,0,255), 2)
                M = cv2.moments(contours[i])
                cx = int(M['m10']/M['m00'])
                cy = int(M['m01']/M['m00'])
                color = cl.label(mask, contours[i])
                contours[i] = contours[i].astype("float")
                contours[i] *= ratio
                contours[i] = contours[i].astype("int")
                cv2.drawContours(resized, [contours[i]], -1, (0, 255, 0), 2)
                cv2.putText(image, color, (cx, cy),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)

            elif(len(approx)>=4 and len(approx)<=6):
                #nb vertices of a polygonal curve
                vtc = len(approx)
                #get cos of all corners
                cos = []
                for j in range(2,vtc+1):
                    cos.append(angle(approx[j%vtc],approx[j-2],approx[j-1]))
                
                cos.sort()
                #get lowest and highest
                mincos = cos[0]
                maxcos = cos[-1]


                x,y,w,h = cv2.boundingRect(contours[i])
                if(vtc==4):
                    cv2.putText(resized,'RECT',(x,y),cv2.FONT_HERSHEY_COMPLEX_SMALL,scale,(255,255,255),2,cv2.LINE_AA)
                    cv2.rectangle(resized,(x,y),(x+w,y+h),(0,0,255), 2)
                    M = cv2.moments(contours[i])
                    cx = int(M['m10']/M['m00'])
                    cy = int(M['m01']/M['m00'])
                    color = cl.label(mask, contours[i])
                    contours[i] = contours[i].astype("float")
                    contours[i] *= ratio
                    contours[i] = contours[i].astype("int")
                    cv2.drawContours(image, [contours[i]], -1, (0, 255, 0), 2)
                    cv2.putText(image, color, (cx, cy),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)


              

            else:
                #circle
                area = cv2.contourArea(contours[i])
                x,y,w,h = cv2.boundingRect(contours[i])
                radius = w/2
                if(abs(1 - (float(w)/h))<=2 and abs(1-(area/(math.pi*radius*radius)))<=0.2):
                    cv2.putText(frame,'CIRC',(x,y),cv2.FONT_HERSHEY_SIMPLEX,scale,(255,255,255),2,cv2.LINE_AA)
                    cv2.rectangle(frame,(x,y),(x+w,y+h),(0,0,255), 2)

        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        red_lower = np.array([136, 87, 111], np.uint8)
        red_upper = np.array([180, 255, 255], np.uint8)

        blue_lower = np.array([110, 50, 50], np.uint8)
        blue_upper = np.array([130, 255, 255], np.uint8)

        yellow_lower = np.array([20,100,100], np.uint8)
        yellow_upper = np.array([30, 255, 255], np.uint8)

        red = cv2.inRange(hsv, red_lower, red_upper)
        blue = cv2.inRange(hsv, blue_lower, blue_upper)
        yellow = cv2.inRange(hsv, yellow_lower, yellow_upper)

        (_, contoursC, hierarchy) = cv2.findContours(blue, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        for pic, contour in enumerate(contoursC):
            area = cv2.contourArea(contour)
            if (area >= 750):
                #Arr.append('Blue')
                x,y,w,h = cv2.boundingRect(contour)
                cv2.putText(resized,'Blue',(x,y),cv2.FONT_HERSHEY_SIMPLEX,0.5,(255,255,255),2,cv2.LINE_AA)


        (_, contoursC, hierarchy) = cv2.findContours(red, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        for pic, contour in enumerate(contoursC):
            area = cv2.contourArea(contour)
            if (area >= 750) :
                #Arr.append('Red')
                x,y,w,h = cv2.boundingRect(contour)
                cv2.putText(resized,'Red',(x,y),cv2.FONT_HERSHEY_SIMPLEX,0.5,(255,255,255),2,cv2.LINE_AA)

        (_, contoursC, hierarchy) = cv2.findContours(yellow, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        for pic, contour in enumerate(contoursC):
            area = cv2.contourArea(contour)
            if (area >= 750) :
                #Arr.append('Yellow')
                x,y,w,h = cv2.boundingRect(contour)
                cv2.putText(resized,'Yellow',(x,y),cv2.FONT_HERSHEY_SIMPLEX,0.5,(255,255,255),2,cv2.LINE_AA)
        cv2.imshow('frame',resized)



    k = cv2.waitKey(8) & 0xFF
    if k == 27:
        break

#When everything done, release the capture
cap.release()
cv2.destroyAllWindows()
