from csv import writer
from datetime import datetime
from os import getpid

import serial

try:
    with serial.Serial('/dev/ttyS7', 9600) as ser:
        tstart = datetime.now()
        while True:
            dt = ser.readline()
            dt = str(dt, encoding='UTF-8').replace('\r\n', '').replace('.', ',')
            tnow = datetime.now()
            t1 = datetime.strftime(tnow, '%H:%M:%S')
            t2 = datetime.strftime(tstart, '%H%M%S')
            data = {t1:dt}
            filename = 'log-' + t2 + '.csv'
            with open(filename, 'a', newline='') as log:
                csvv = writer(log, delimiter = ';')
                for value in data.items():
                    csvv.writerow(value)
except KeyboardInterrupt:
    print('\nPrograma concluído.')
    print('Arquivo: {0}'.format(filename))
except serial.serialutil.SerialException:
    print('\nNão foi possível realizar a conexão.')
except Exception as ex:
    print(ex)