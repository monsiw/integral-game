import kivy
kivy.require('1.9.0')
from kivy.config import Config
Config.set('graphics', 'resizable', '0') 
Config.set('graphics', 'width', '800')
Config.set('graphics', 'height', '450')
from kivy.app import App
from kivy.uix.widget import Widget
from kivy.uix.image import Image
from kivy.core.window import Window
from kivy.clock import Clock
from kivy.core.audio import SoundLoader
import kivy.core.text.markup
import random
import paho.mqtt.client as mqtt
import threading


class Background(Widget):    
    cloud_texture = ObjectProperty(None)
    floor_texture = ObjectProperty(None)
	
    def __init__(self,**kwargs): 
        super().__init__(**kwargs)        
        self.cloud_texture = Image(source = "cloud.png").texture
        self.cloud_texture.wrap = 'repeat'
        self.cloud_texture.uvsize = (Window.width/self.cloud_texture.width, -1)
        self.floor_texture = Image(source = "grass.png").texture
        self.floor_texture.wrap = 'repeat'
        self.floor_texture.uvsize = (Window.width/self.floor_texture.width, -1)

    def scroll_texture(self,step_size):
        self.cloud_texture.uvpos = ((self.cloud_texture.uvpos[0]-step_size/3)%Window.width,self.cloud_texture.uvpos[1])
        texture=self.property('cloud_texture')
        texture.dispatch(self)    
        
       
class Cat(Image): 
    pass
class Food(Image):
    pass
class Sun(Image):
    pass  
class Label(Image):
    pass 


msg_status=0
      

class mymainapp(App):
    # sound = SoundLoader.load('Optimistic-background-music.wav')
    # if sound:
    #     sound.play()

    def connect_mqtt(self) -> mqtt:
        broker = '192.168.56.1'
        port = 1883       
        client_id = f'subscribe-{random.randint(0, 100)}'
        username = 'monsiw'
        password = 'haslo'
        def on_connect(client, userdata, flags, rc):
            if rc == 0:
                print("Connected to MQTT Broker!")
            else:
                print("Failed to connect, return code %d\n", rc)
        client = mqtt.Client(client_id)
        client.username_pw_set(username, password)
        client.on_connect = on_connect
        client.connect(broker, port)
        return client
    
    def subscribe(self, client: mqtt, topic):
        topic = "python/mqtt"       
        def on_message(client, userdata, msg):    
            global msg_status
            print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
            msg_status = int(msg.payload.decode())
        client.subscribe(topic)
        client.on_message = on_message  

    def connection_mqtt(self):
        topic = "python/mqtt"
        client = self.connect_mqtt()
        self.subscribe(client,topic)
        client.loop_forever()

    def start_game(self):    
        t1=threading.Thread(target=self.connection_mqtt, daemon=True)
        t1.start()           
        Clock.schedule_interval(self.next_frame, 1/60.)  
     
    def move_step(self, dt):
        global msg_status
        cat = self.root.ids.cat.pos[0]      
        step_size = 200 * dt
        if msg_status==1: 
            cat += step_size
            self.root.ids.cat.source = "cat2.png"
            self.root.ids.cat.size = (330,330)
            self.root.ids.cat.pos[1] = -60
            self.root.ids.label1.text = "[color=#a12c3c][/color]"   
        else:
            self.root.ids.cat.source = "cat1.png"
            self.root.ids.cat.size = (270,270)
            self.root.ids.label1.text = "[color=#a12c3c]DMUCHAJ![/color]"        
        self.root.ids.cat.pos[0] = cat 
    
    def next_frame(self, step_size):   
        self.move_step(step_size)
        if(self.root.ids.cat.pos[0]>=self.root.ids.food.pos[0]):
            self.root.ids.label1.text = "[color=#a12c3c]WYGRANA![/color]"
        self.root.ids.background.scroll_texture(step_size)   
        
        
if __name__=="__main__":        
    mymainapp().run()