# -*- coding: utf-8 -*-

"""
Python tools for EMG reading

Bryn Davis
Created: 12 Feb 2019
"""

import numpy as np

def read_flexvolt(filename):
    """Reads data files saved by the FlexVolt Chrome plugin
    
    """
    
    # Get the header and column headings
    with open(filename, 'r') as f:
        header_text = f.readline()
        header_text = header_text.replace('false', 'False')
        header_text = header_text.replace('true', 'True')
        header = eval(header_text)
        columns = f.readline()
        columns = columns.lower().split(',')

    # Load the data
    temp = np.loadtxt(filename, skiprows=2, delimiter=',')
    time = temp[:, np.argwhere(['time' in cl for cl in columns])].squeeze()
    raw_data = temp[:, np.argwhere(['raw' in cl for cl in columns])].squeeze()
    proc_data = temp[:, np.argwhere(['process' in cl for cl in columns])].squeeze()
    
    return proc_data, raw_data, time, header
    
if __name__ == '__main__':
    
    import os
    from matplotlib import pyplot as plt
    
    # Basic debug and testing
    
    g_drive_path = '/Users/Bryn/Google Drive/Robotic hand/EMG Tests/Data'  # Edit based on your machine
    #fn = 'Bryn/12-2-19/shaved_forearm_dry1_and_wet2_shared_wet_ref.txt'
    #fn = 'Alquimedes Celda/12-2-19/flexvolt-recorded-data--2019-02-12--13-55-34.txt'
    #fn = 'Alquimedes Celda/12-2-19/flexvolt-recorded-data--2019-02-12--14-09-53.txt'
    #fn = 'Alquimedes Celda/12-2-19/flexvolt-recorded-data--2019-02-12--14-14-12.txt'
    fn = 'Alquimedes Celda/12-2-19/flexvolt-recorded-data--2019-02-12--14-16-26.txt'
    
    proc_data, raw_data, time, header = read_flexvolt(os.path.join(g_drive_path, fn))

    plt.figure(0)
    plt.plot(time, proc_data)
    plt.figure(1)
    plt.plot(time, raw_data)
    plt.legend(['Channel 1', 'Channel 2', 'Channel 3', 'Channel 4'])
    plt.show()