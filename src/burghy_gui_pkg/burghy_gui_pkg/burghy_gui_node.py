import threading
import signal
from pathlib import Path

import rclpy
from rclpy.node import Node

from std_msgs.msg import String

from rclpy.qos import qos_profile_sensor_data
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy
from rclpy.executors import ExternalShutdownException
from nicegui import ui, app, ui_run


# =========================
# Shared GUI/ROS state
# =========================

node = None

heartbeat_enabled = True
reset_request_received = False

question = None
response = None 
last_response = None

cone_data = {
    1: {},
    2: {},
    3: {},
    4: {},
}

cone_widgets = {}

STATUS_COLORS = {
    0: 'white',
    1: 'blue',
    2: 'green',
    3: 'orange',
    4: 'red',
}

STATUS_TEXT = {
    0: 'None',
    1: 'Setup',
    2: 'Operational',
    3: 'Local Alarm',
    4: 'Global Alarm',
}


# =========================
# ROS2 Node
# =========================

class GuiNode(Node):

    def __init__(self):
        super().__init__('gui_node')

        self.publisher_ = self.create_publisher(
            String,
            'ollama/request',
            10
        )

        self.subscription = self.create_subscription(
            String,
            'ollama/response',
            self.listener_callback,
            10
        )

        #self.create_timer(0.3, self.heartbeat_timer)

    def listener_callback(self, msg: String):
        global response
        response = msg.data



# =========================
# ROS thread
# =========================

def ros_main():
    global node

    rclpy.init()
    node = GuiNode()

    try:
        rclpy.spin(node)

    except (KeyboardInterrupt, ExternalShutdownException):
        pass

    finally:
        if node is not None:
            node.destroy_node()

#real startup    

@app.on_startup
def startup():
    threading.Thread(
        target=ros_main,
        daemon=True,
    ).start()


# =========================
# GUI helpers
# =========================

def send_reset():
    global reset_request_received
    reset_request_received = False
    if node is None:
        ui.notify('ROS node not ready', color='negative')
        return

    #msg = Bool()
    #msg.data = True

    #node.publisher_reset_cmd.publish(msg)
    ui.notify('Reset command sent')


def send_alarm(level):
    if node is None:
        ui.notify('ROS node not ready', color='negative')
        return

    #msg = UInt8()
    #msg.data = int(level)

    #node.publisher_alarm.publish(msg)
    ui.notify(f'Alarm level {level} sent')


def heartbeat_changed(event):
    global heartbeat_enabled
    heartbeat_enabled = bool(event.value)


class ConeWidget:

    def __init__(self, cone_id: int):
        self.cone_id = cone_id

        with ui.card().classes('items-center').style('width: 250px;'):
            self.title = ui.label(f'Cone ID: {cone_id}').classes('text-h5')

            self.light = ui.element('div').classes(
                'rounded-full shadow-lg'
            ).style(
                '''
                width: 120px;
                height: 120px;
                background-color: blue;
                '''
            )

            self.status = ui.label('Status: --')
            self.expected = ui.label('Expected: -- cm')
            self.measured = ui.label('Measured: -- cm')
            self.deviation = ui.label('Deviation: -- cm')
            self.intensity = ui.label('Intensity: --')
            self.temperature = ui.label('Temperature: -- °C')
            self.timestamp = ui.label('Timestamp: --')

    def update(self, data: dict):
        status = data.get('status', 0)
        color = STATUS_COLORS.get(status, 'gray')
        status_text = STATUS_TEXT.get(status, 'Unknown')

        self.light.style(
            f'''
            width: 120px;
            height: 120px;
            background-color: {color};
            '''
        )

        self.status.set_text(f'Status: {status} - {status_text}')
        self.expected.set_text(f'Expected: {data.get("expected", "--")} cm')
        self.measured.set_text(f'Measured: {data.get("measured", "--")} cm')

        deviation = data.get('deviation', '--')
        if isinstance(deviation, float):
            deviation = f'{deviation:.2f}'

        self.deviation.set_text(f'Deviation: {deviation} cm')
        self.intensity.set_text(f'Intensity: {data.get("intensity", "--")}')
        self.temperature.set_text(f'Temperature: {data.get("temperature", "--")} °C')

        if 'stamp_sec' in data:
            self.timestamp.set_text(
                f'Timestamp: {data["stamp_sec"]}.{data["stamp_nsec"]:09d}'
            )
        else:
            self.timestamp.set_text('Timestamp: --')


# =========================
# NiceGUI page
# =========================

@ui.page('/')
def page():
    with ui.column().classes('w-full items-center'):

        ui.label('BURGER ROBOT').classes('text-h4')

        with ui.card().classes('w-full'):
            ui.label('Control Panel').classes('text-h5')

            with ui.row().classes('items-center'):
                ui.button(
                    'Send Reset',
                    on_click=send_reset,
                )

                ui.switch(
                    'Heartbeat',
                    value=True,
                    on_change=heartbeat_changed,
                )

                alarm_level = ui.select(
                    [0, 1, 2, 3],
                    value=0,
                    label='Alarm level',
                ).style('width: 180px;')

                ui.button(
                    'Send Alarm',
                    on_click=lambda: send_alarm(alarm_level.value),
                )

            heartbeat_label = ui.label()
            reset_request_label = ui.label()

        message_container = (
                ui.column()
                .classes('w-full max-w-2xl mx-auto flex-grow items-stretch')
            )   

    async def send():

        question = text.value.strip()
        msg = String()
        msg.data = question

        node.publisher_.publish(msg)

        if not question:
            return

        text.value = ''
        message_container.clear()
        with message_container:
            ui.chat_message(
                text=question,
                name='You',
                sent=True
            )


        ui.run_javascript(
            'window.scrollTo(0, document.body.scrollHeight)'
        )
    #input text chat
    with ui.footer().classes('bg-white'):
        with ui.row().classes('w-full no-wrap items-center'):

            text = (
                ui.input(
                    placeholder='Write a message...'
                )
                .classes('w-full')
                .on('keydown.enter', send)
            )

            ui.button(
                'Send',
                on_click=send
            )

        def update_gui():
            global response
            global last_response

            heartbeat_label.set_text(
                f'Heartbeat: {"ENABLED" if heartbeat_enabled else "DISABLED"}'
            )

            reset_request_label.set_text(
                f'Reset request received: {reset_request_received}'
            )
            if response is not None and response != last_response:

                last_response = response

                with message_container:
                    ui.chat_message(
                        text=response,
                        name='Burghy',
                        sent=False
                    )

        ui.timer(0.2, update_gui)


def main():
    pass

cleanup_done = False

def cleanup():
    global cleanup_done

    if cleanup_done:
        return

    cleanup_done = True

    print('Cleaning up...')

    if rclpy.ok():
        rclpy.shutdown()


@app.on_shutdown
def shutdown():
    cleanup()


ui_run.APP_IMPORT_STRING = f'{__name__}:app'
ui.run(
    uvicorn_reload_dirs=str(Path(__file__).parent.resolve()),
)