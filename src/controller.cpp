#include <thread>
#include <iostream>
#include <stdexcept>
#include <atomic>
//#include <csignal>

#include "controller.h"

using std::chrono::system_clock;
using std::chrono::milliseconds;
using std::string;
using std::vector;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;


const string Controller::cv_window_name = "Tello camera";

void Controller::get_battery_stat() {
	//bool stop;
	while (true) {
		//unique_lock();
		//stop = stop_control_thread;
		//unique_lock.stop();
		//if (stop){
			//break;
		//}
		std::this_thread::sleep_for(milliseconds(Controller::WAIT_BATTERY));
		this->battery_stat = this->tello.get_battery();
	}
}

void Controller::run() {
	// log
	//atomic<bool> stop_control_thread = false;
	// TODO check number of threads
	std::thread control_thread(&Controller::send_command, this);
	control_thread.detach();
	//atomic<bool> stop_battery_thread = false;
	std::thread battery_thread(&Controller::get_battery_stat, this);
	battery_thread.detach();

	// log
	cv::Mat image;
	cv::namedWindow(Controller::cv_window_name);
	cv::VideoCapture cap(0);
	if (!cap.isOpened()) {
		// log
		throw std::exception("Unable to open video stream");
	}
	while (true) {
		cap >> image;
		//image = this->detection_step();
		cv::imshow(Controller::cv_window_name, image);
		if (cv::waitKey(25) == 27 ||
			cv::getWindowProperty(Controller::cv_window_name, cv::WND_PROP_VISIBLE) < 1
			) {
			cv::destroyAllWindows();
			// TODO threads clean-up
			//mutex.lock();
			//stop_battery_thread = true;
			//mutex.unlock();

			// TODO tello clean-up
			break;
		}

	}
	// TODO catch Ctrl+C KeyboardInterrupt (<csignal>?)
}

//        try:
//            while True:
//                img = self.frame_read.frame
//                if img is None:
//                    continue
//                img = self.step(img)
//                if self.save_video:
//                    writer.write(img)
//                cv2.imshow(self.CAMERA_NAME, img)
//
//                if cv2.waitKey(1) == 27 or (
//                    int(
//                        cv2.getWindowProperty(
//                            self.CAMERA_NAME, cv2.WND_PROP_VISIBLE
//                        )
//                    )
//                    < 1
//                ):
//                    timer_bat.cancel()
//                    timer_com.cancel()
//                    if not self.debug:
//                        self.tello.land()
//                    if self.save_video:
//                        writer.release()
//                    self.tello.streamoff()
//                    break
//        except KeyboardInterrupt:
//            self.logger.info("KeyboardInterrupt: exiting")
//            timer_bat.cancel()
//            timer_com.cancel()
//            if not self.debug:
//                self.tello.land()
//            if self.save_video:
//                        writer.release()
//            self.tello.streamoff()

void Controller::detection_step(cv::Mat * img) {
	this->_put_battery_on_frame(img);
	// TODO implement detection logic
}

void Controller::send_command() {
	while (true) {
		std::this_thread::sleep_for(milliseconds(Controller::WAIT_RC_CONTROL));

		if (!this->stop_tello) {
			if ((system_clock::now() - this->_last_face) > this->FACE_TIMEOUT ||
				(system_clock::now() - this->_last_gesture) > this->GESTURE_TIMEOUT){
					this->stop();
			}
			else {
				string gesture = this->buffer.get();
				if (!gesture.empty()) {
					// log
				}
				if (!this->is_landing) {
					vector<int> vel = {-1, -1, -1, -1};
				}
				if (vel.at(3) != -1) {
					// log
					this->vel = vel;
					if (!this->debug) {
						this->tello.send_rc_control(vel);
					}
				}
			}
		}
		//                    if gesture == Gesture.FRONT:
		//                        vel = [0, self.DW[1], 0, 0]
		//                    elif gesture == Gesture.BACK:
		//                        vel = [0, -self.DW[1], 0, 0]
		//                    elif gesture == Gesture.LEFT:
		//                        vel = [-self.DW[0], 0, 0, 0]
		//                    elif gesture == Gesture.RIGHT:
		//                        vel = [self.DW[0], 0, 0, 0]
		//                    elif gesture == Gesture.UP:
		//                        vel = [0, 0, self.DW[2], 0]
		//                    elif gesture == Gesture.DOWN:
		//                        vel = [0, 0, -self.DW[2], 0]
		//                    elif gesture == Gesture.STOP:
		//                        vel = [0, 0, 0, 0]
		//                    elif gesture == Gesture.FLIP:
		//                        self.tello.flip_back()
	}
}

void Controller::stop() {
	this->vel = { 0, 0, 0, 0 };
	this->stop_tello = true;
	this->tello.send_rc_control(this->vel);
}

void Controller::_put_battery_on_frame(cv::Mat* img) {
	string text = std::to_string(this->battery_stat);
	cv::putText(*img, text, cv::Point(20, 100), 1, 2, (0, 255, 255), 2);
}

template<class T>
void Buffer<T>::add(const T &elem) {
	this->_buffer.push_back(elem);
}

template <class T>
T Buffer<T>::get() {
	if (this->_buffer.empty()) {
		T val{};
		return val;
	}
	else {
		// TODO actual buffer
		return _buffer.at(0);
	}
}

void Tello::send_rc_control(const vector<int> &vel) {
	// log
}

// TODO write Frameread

// TODO write DummyTello


//class GestureBuffer:
//    """A deque-based buffer to ignore accidental gestures"""
//
//    def __init__(self, buffer_len: int = 10) -> None:
//        self.buffer_len = buffer_len
//        self._buffer: deque = deque(maxlen=buffer_len)
//
//    def add_gesture(self, gesture: Gesture) -> None:
//        self._buffer.append(gesture)
//
//    def get_gesture(self) -> Gesture | None:
//        """Return the most common of the last buffer_len gestures"""
//        counter = Counter(self._buffer).most_common()
//        if counter and counter[0][1] >= (self.buffer_len - 1):
//            self._buffer.clear()
//            return counter[0][0]
//        else:
//            return None
//
//

//class CustomFrameRead:
//    def __init__(self, camera: str | int) -> None:
//        self.cap = cv2.VideoCapture(camera)
//
//    @property
//    def frame(self) -> ndarray | None:
//        _, frame = self.cap.read()
//        return frame
//
//    def end(self) -> None:
//        self.cap.release()
//        cv2.destroyAllWindows()
//


//class DummyTello:
//    """Imitates Tello to test TelloDetectionRunner"""
//
//    def __init__(self) -> None:
//        self.battery = 50
//        self.background_frame_read: CustomFrameRead | None = None
//        self.camera = 0
//        self.logger = logging.getLogger(__name__)
//        self.streaming = False
//
//    def connect(self) -> None:
//        self.logger.info("Connected Tello")
//
//    def streamon(self) -> None:
//        self.streaming = True
//        self.logger.info("Stream is on")
//
//    def streamoff(self) -> None:
//        if self.streaming:
//            self.streaming = False
//            if self.background_frame_read is not None:
//                self.background_frame_read.end()
//            self.logger.info("Stream is off")
//
//    def takeoff(self) -> None:
//        self.logger.info("Taking off")
//
//    def move_up(self, up: int) -> None:
//        self.logger.info(f"up {up}")
//
//    def flip_back(self) -> None:
//        self.logger.info("Flip XD")
//
//    def land(self) -> None:
//        self.logger.info("Landed")
//
//    def send_rc_control(self, x: int, y: int, z: int, yaw: int) -> None:
//        self.logger.info(f"rc {x, y, z, yaw}")
//
//    def get_battery(self) -> int:
//        return self.battery
//
//    def get_frame_read(self) -> CustomFrameRead:
//        if self.background_frame_read is None:
//            self.background_frame_read = CustomFrameRead(self.camera)
//        return self.background_frame_read


//class TelloDetectionRunner:
//    def __init__(
//        self,
//        tello: Tello,
//        buffer_len: int = 5,
//        debug: bool = True,
//        DW: List = [10, 10, 10],
//        detection_logging_level: int = logging.INFO,
//        controller_logging_level: int = logging.DEBUG,
//        save_video: bool = False,
//    ) -> None:
//        self.tello = tello
//        self.debug = debug
//        self.Buffer = GestureBuffer(buffer_len)
//        self.FaceDetector = MTCNNDetector(
//            logging_level=detection_logging_level
//        )
//        self.GestureDetector = MediaPipeGestureDetector(
//            logging_level=detection_logging_level
//        )
//        self.logger = logging.getLogger(__name__)
//        self.logger.setLevel(controller_logging_level)
//        self._battery_stat = -1
//        self._last_face = -1.0
//        self._last_gesture = -1.0
//        self._stop_tello = False
//        self._is_landing = False
//        self.save_video = save_video
//
//        # constants
//        self.WAIT_RC_CONTROL = 0.5
//        self.WAIT_BATTERY = 4
//        self.FACE_TIMEOUT = 2
//        self.GESTURE_TIMEOUT = 2
//        self.CAMERA_NAME = "Tello-camera"
//        self.vel = [0, 0, 0, 0]  # [left-right, front-back, up-down, yaw]
//        self.DW = DW
//
//        self.frame_read = self.tello.get_frame_read()
//
//        if not self.debug:
//            self.tello.takeoff()
//            self.tello.move_up(90)


//    def run(self) -> None:
//        """Perform tello detection and control main loop."""
//
//        # Start background processes in separate threads
//        self.logger.info("Starting background processes")
//        timer_com = RepeatTimer(self.WAIT_RC_CONTROL, self.control_tello)
//        timer_com.start()
//
//        timer_bat = RepeatTimer(self.WAIT_BATTERY, self._get_battery)
//        timer_bat.start()
//
//        self.logger.info("Starting detection")
//        if self.save_video:
//                    width = 640
//                    height = 480
//                    writer = cv2.VideoWriter(
//                        'video.avi',
//                        0,
//                        10, (width,height))
//
//        try:
//            while True:
//                img = self.frame_read.frame
//                if img is None:
//                    continue
//                img = self.step(img)
//                if self.save_video:
//                    writer.write(img)
//                cv2.imshow(self.CAMERA_NAME, img)
//
//                if cv2.waitKey(1) == 27 or (
//                    int(
//                        cv2.getWindowProperty(
//                            self.CAMERA_NAME, cv2.WND_PROP_VISIBLE
//                        )
//                    )
//                    < 1
//                ):
//                    timer_bat.cancel()
//                    timer_com.cancel()
//                    if not self.debug:
//                        self.tello.land()
//                    if self.save_video:
//                        writer.release()
//                    self.tello.streamoff()
//                    break
//        except KeyboardInterrupt:
//            self.logger.info("KeyboardInterrupt: exiting")
//            timer_bat.cancel()
//            timer_com.cancel()
//            if not self.debug:
//                self.tello.land()
//            if self.save_video:
//                        writer.release()
//            self.tello.streamoff()

//    def step(self, img: ndarray) -> ndarray:
//        """Perform detection on an image."""
//        img = self._put_battery_on_frame(img)
//
//        face, score = self.FaceDetector.detect(img)
//        if face and score:
//            self._last_face = time.time()
//            mask, lt, rb = get_mask(face, img)
//            masked = cv2.bitwise_and(img, img, mask=mask)
//            gesture, hand = self.GestureDetector.detect(masked)
//
//            img = self.FaceDetector.visualize(img, face, score)
//            if gesture and hand:
//                self._last_gesture = time.time()
//                self._stop_tello = False
//                self.Buffer.add_gesture(gesture)
//                img = self.GestureDetector.visualize(img, gesture, hand)
//            img = draw_bounds(img, lt, rb)
//        return img

//    def _get_battery(self) -> None:
//        self._battery_stat = self.tello.get_battery()
//        self.logger.debug(f"Battery percentage: {self._battery_stat}%")

//    def _put_battery_on_frame(self, img: ArrayLike) -> ndarray:
//        text = f"{self._battery_stat}%"
//        return cv2.putText(img, text, (20, 100), 1, 2, (0, 255, 255), 2)


//    def control_tello(self) -> None:
//        """Send the next command in the buffer to the Tello instance."""
//        if not self._stop_tello:
//            timed_out = False
//            if self._last_face > 0:
//                timed_out = (time.time() - self._last_face) > self.FACE_TIMEOUT
//            if self._last_gesture > 0:
//                timed_out = timed_out or (
//                    (time.time() - self._last_gesture) > self.GESTURE_TIMEOUT
//                )
//
//            # if a face or a gesture have not been detected for a set amount
//            # of time, stop tello
//            if timed_out:
//                self.logger.info("No face or no gesture - tello stopped")
//                self.stop()
//                self._stop_tello = True
//            else:
//                gesture = self.Buffer.get_gesture()
//                if gesture is not None:
//                    self.logger.debug(f"Received command: {gesture}")
//
//                if not self._is_landing:
//                    vel = None
//                    if gesture == Gesture.FRONT:
//                        vel = [0, self.DW[1], 0, 0]
//                    elif gesture == Gesture.BACK:
//                        vel = [0, -self.DW[1], 0, 0]
//                    elif gesture == Gesture.LEFT:
//                        vel = [-self.DW[0], 0, 0, 0]
//                    elif gesture == Gesture.RIGHT:
//                        vel = [self.DW[0], 0, 0, 0]
//                    elif gesture == Gesture.UP:
//                        vel = [0, 0, self.DW[2], 0]
//                    elif gesture == Gesture.DOWN:
//                        vel = [0, 0, -self.DW[2], 0]
//                    elif gesture == Gesture.STOP:
//                        vel = [0, 0, 0, 0]
//                    elif gesture == Gesture.FLIP:
//                        self.tello.flip_back()
//
//                if vel and (self.vel != vel):
//                    self.logger.debug(f"Setting speed to: {vel}")
//                    self.vel = vel
//                    if not self.debug:
//                        self.tello.send_rc_control(*self.vel)

//    def stop(self) -> None:
//        """Set tello velocity to zero"""
//        self.vel = [0, 0, 0, 0]
//        self.tello.send_rc_control(*self.vel)
