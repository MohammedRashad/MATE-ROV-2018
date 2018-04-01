import cv2
import numpy as np
import pytesseract
import os
from PIL import Image

cap = cv2.VideoCapture(0)

while 1:
    _, img = cap.read()
    #Text Detection
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)[1]
    gray = cv2.medianBlur(gray, 3)
    temp = 'temp.png'
    cv2.imwrite(temp, gray)
    text = pytesseract.image_to_string(Image.open(temp))
    os.remove(temp)
    if text == 'UHB' :
            print('A')
    elif text == 'LER':
            print('B')
    elif text == 'C {C' :
            print ('C')
    elif text == 'SIP':
            print ('D')
    elif text == 'JWB':
            print('E')
    elif text == 'AZX':
            print ('F')
    print (text)
    cv2.imshow('cont', img)
      
    k = cv2.waitKey(8) & 0xFF
    if k == 27:
        break
cap.release()
cv2.destroyAllWindows()
