import cv2
import matplotlib.pyplot as plt

imagePath = r'D:\data\face_dataset\self_selected_images\00.jpg'
img = cv2.imread(imagePath)
gray_image = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

face_classifier = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")

face = face_classifier.detectMultiScale(gray_image, scaleFactor=1.1, minNeighbors=5, minSize=(40, 40))
# scaleFactor：此参数用于缩小输入图像的尺寸，以便算法更容易检测较大的人脸。在本例中，我们指定了比例因子 1.1，表示我们希望将图像尺寸缩小 10%。
# minNeighbors：级联分类器在图像中应用滑动窗口来检测其中的人脸。您可以将这些窗口视为矩形。最初，分类器会捕获大量误报。这些误报可通过参数消除minNeighbors，该参数指定了需要识别的相邻矩形的数量，才能将对象视为有效检测。总而言之，将 0 或 1 这样的小值传递给此参数会导致大量的误报，而大量的误报可能会导致失去许多真正报。
# 这里的诀窍是找到一种权衡，让我们能够消除假阳性，同时还能准确识别真阳性。
# minSize：最后，该minSize参数设置要检测的物体的最小尺寸。模型将忽略小于指定最小尺寸的人脸。
for (x, y, w, h) in face: cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 0), 4)
# img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

cv2.imshow("img_rgb",img)
cv2.waitKey(0)
cv2.destroyAllWindows()
# plt.figure(figsize=(20,10))
# plt.imshow(img_rgb)
# plt.axis('off')