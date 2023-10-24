import cv2
import urllib.request
from ps4_controller import PS4Controller

root_url = "http://192.168.0.18"
setEyeCommand = "SET_EYE"
setLidsCommand = "SET_LIDS"

ps4 = PS4Controller()
ps4.init()
ps4.listen()

face_classifier = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
)

video_capture = cv2.VideoCapture(1)
captureWidth = video_capture.get(cv2.CAP_PROP_FRAME_WIDTH)


def on_hat_callback(data):
    print(data)
    if 2 not in data or 3 not in data:
        return

    sendEyePitchYaw(data[2], data[3])


ps4.register_callback(on_hat_callback)


def sendToESP(request):
    sendRequest(root_url + "/" + request)


def sendRequest(url):
    print(url)
    n = urllib.request.urlopen(url)  # send request to ESP


def sendEyePitchYaw(pitch, yaw):
    sendToESP(f"{setEyeCommand}/{pitch}/{yaw}")


def sendLidsPosition(value):
    sendToESP(f"{setLidsCommand}/{value}")


def detect_bounding_box(vid):
    gray_image = cv2.cvtColor(vid, cv2.COLOR_BGR2GRAY)
    faces = face_classifier.detectMultiScale(gray_image, 1.1, 5, minSize=(40, 40))
    for x, y, w, h in faces:
        cv2.rectangle(vid, (x, y), (x + w, y + h), (0, 255, 0), 4)
        centerX = round(x + w / 2)
        centerY = round(y + h / 2)
        cv2.circle(vid, (centerX, centerY), 3, (255, 0, 0), 4)
    return faces


while True:
    ps4.listen()
#     result, video_frame = video_capture.read()  # read frames from the video
#     if result is False:
#         break  # terminate the loop if the frame is not read successfully

#     faces = detect_bounding_box(
#         video_frame
#     )  # apply the function we created to the video frame

#     if len(faces) > 0:
#         mainFace = faces[0]
#         aimingNormalized = (mainFace[0] + (mainFace[2] / 2)) / captureWidth
#         sendToESP("SET_X/" + str(aimingNormalized))

#     cv2.imshow(
#         "My Face Detection Project", video_frame
#     )  # display the processed frame in a window named "My Face Detection Project"

#     if cv2.waitKey(1) & 0xFF == ord("q"):
#         break

# video_capture.release()
# cv2.destroyAllWindows()
