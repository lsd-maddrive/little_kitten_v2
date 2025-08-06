import json
import serial
from time import sleep
from pygamepad.gamepads import Gamepad


ser = serial.Serial('COM5',115200)  

#Максимальная скорость без нагрузки 1444 об/мин = 0.3 м/с
#Максимальная скорость под нагрузкой 720 об/мин = 0.16 м/с
#Радиус колеса 0,067 м

def main():
    gamepad = Gamepad()
    gamepad.listen()  # And that's it

    # Now you can read gamepad values in the main thread
    try:
        while True:
 
            R = -round(gamepad.Buttons.ABS_Y.value,2)*0.16
            if (R<=0.05 and R>=-0.05): R=0;
            L = -round(gamepad.Buttons.ABS_RY.value,2)*0.16
            if (L<=0.05 and L>=-0.05): L=0;
            
            #L = -int(round(gamepad.Buttons.ABS_RY.value,2)*250)
            #if (L<=7 and L>=-7): L=0;
            data = { 
                "Motor":{
                    "Vr": R,
                    "Vl": L
                }
            }
            json_str = json.dumps(data)
            print(json_str.encode('utf-8'))
            ser.write(json_str.encode('utf-8'))
            #ln = ser.read_()
            recv = ser.read_until(b'\r\n')[:-2]
            recv_data = ""
            #print(recv)
            if recv == b'{':
                #print(recv)
                recv_data = recv
                while recv != b'}':
                    recv = ser.read_until(b'\r\n')[:-2]
                    #print(recv)
                    recv_data+=recv
                ser.read_all()
                #print(str(recv_data.decode("utf-8")).replace(" ",""))

            if recv_data!="":
                try:
                    recv_json = json.loads(str(recv_data.decode("utf-8")).replace(" ",""))
                    R_spd = recv_json["enc"]["Right"]
                    L_spd = recv_json["enc"]["Left"]
                    #print(f"Right: {R_spd}, Left: {L_spd}")
                except:
                    pass

            sleep(0.01)
            
    except (KeyboardInterrupt, SystemExit):
        # Kill gamepad's listening thread
        gamepad.stop_listening()
        # And exit from the program
        exit()


if __name__ == "__main__":
    main()