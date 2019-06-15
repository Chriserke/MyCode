package com.christiaan.bluetoothhc05;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.christiaan.bluetoothhc05.graphs.LineChartActivity;
import com.example.danyal.bluetoothhc05.R;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.UUID;


public class LedControl extends AppCompatActivity {
    public static ArrayList<HeartbeatData> heartbeatDataList = new ArrayList<>();
    public static ArrayList<GeleidingSensor> geleidingSensorArrayList = new ArrayList<>();
    public static ArrayList<DrukSensorData> drukSensorArrayList = new ArrayList<>();

    public class GeleidingSensor {
        public GeleidingSensor(double conduction, long timestamp) {
            this.conduction = conduction;
            this.timestamp = timestamp;
        }
        public double conduction;
        public long timestamp;
    }

    public class HeartbeatData {
        public HeartbeatData(int heartbeat, long timestamp) {
            this.heartbeat = heartbeat;
            this.timestamp = timestamp;
        }
        public int heartbeat;
        public long timestamp;
    }

    public class DrukSensorData {
        public DrukSensorData(double druk, long timestamp) {
            this.druk = druk;
            this.timestamp = timestamp;
        }
        public double druk;
        public long timestamp;
    }

    Button btn1, btn2, btn3, btn4, btn5, btnDis, btnShowData;
    String address = null;
    TextView lumn;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    private boolean isBtConnected = false;
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        address = newint.getStringExtra(DeviceList.EXTRA_ADDRESS);

        setContentView(R.layout.activity_led_control);

        btn1 = (Button) findViewById(R.id.button2);
        btn2 = (Button) findViewById(R.id.button3);
        btn3 = (Button) findViewById(R.id.button5);
        btn4 = (Button) findViewById(R.id.button6);
        btn5 = (Button) findViewById(R.id.button7);
        btnDis = (Button) findViewById(R.id.button4);
        btnShowData = (Button) findViewById(R.id.btnGraph);
        lumn = (TextView) findViewById(R.id.textView2);

        new ConnectBT().execute();

        btn1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendSignal("1");
            }
        });

        btn2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendSignal("2");
            }
        });

        btn3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendSignal("3");
            }
        });

        btn4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendSignal("4");
            }
        });

        btn5.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendSignal("5");
            }
        });

        btnDis.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Disconnect();
            }
        });

        btnShowData.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(LedControl.this, LineChartActivity.class));
            }
        });

        new Thread(new Runnable() {
            public void run() {
                Looper.prepare();
                while(true) {
                    readSignal();
                }
            }
        }).start();


    }

    private void sendSignal(String number) {
        if (btSocket != null) {
            try {
                btSocket.getOutputStream().write(number.getBytes());
            } catch (IOException e) {
                msg("Error");
            }
        }
    }

    private void readSignal() {
        try {
            if (btSocket != null && btSocket.isConnected()) {
                int bufferLength = 2048;
                byte[] buffer = new byte[bufferLength];
                int offset = 0;
                while (btSocket.getInputStream().available() > 0) {
                    offset += btSocket.getInputStream().read(buffer, offset, bufferLength - offset);
                    Thread.sleep(1);
                }
                if(offset > 0) {
                    handlePacket(buffer, offset);
                }
            }
        } catch (IOException e) {
            msg("Error reading input.");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }


    byte firstByte = 0;
    Byte lastPayloadByte = null;

    private void handlePacket(byte[] packet, int length) {
        for (byte b : packet) {
            int positiveByte = b & 0xFF;
            if (firstByte == 1 && lastPayloadByte != null) {
                // waarde 1 - Heartbeat
                heartbeatDataList.add(new HeartbeatData((lastPayloadByte << 8) + positiveByte, Calendar.getInstance().getTimeInMillis()));

                firstByte = 0;
                lastPayloadByte = null;
            } else if (firstByte == 2 && lastPayloadByte != null) {
                // waarde 2
                geleidingSensorArrayList.add(new GeleidingSensor((lastPayloadByte << 8) + positiveByte, Calendar.getInstance().getTimeInMillis()));

                firstByte = 0;
                lastPayloadByte = null;
            } else if (firstByte == 3 && lastPayloadByte != null) {
                // waarde 3
                drukSensorArrayList.add(new DrukSensorData((lastPayloadByte << 8) + positiveByte, Calendar.getInstance().getTimeInMillis()));

                firstByte = 0;
                lastPayloadByte = null;
            } else if (firstByte == 0 && (b == 1 || b == 2 || b == 3) ) {
                firstByte = b;
                lastPayloadByte = null;
            } else {
                lastPayloadByte = b;
            }
        }
        final String payload = new String(packet, 0, length);
        Handler mainHandler = new Handler(Looper.getMainLooper()); // Get the handler linked to the main thread.
        Runnable runnable = new Runnable() { // Define the UI modification.
            @Override
            public void run() {
                TextView tv = (TextView) findViewById(R.id.Tv);
                tv.append("Received: " + payload + "\n");
            }
        };
        // Add it to the main thread execution queue so it will be run at the main thread.
        mainHandler.post(runnable);
    }

    private void Disconnect() {
        if (btSocket != null) {
            try {
                btSocket.close();
            } catch (IOException e) {
                msg("Error");
            }
        }

        finish();
    }

    private void msg(String s) {
        Toast.makeText(getApplicationContext(), s, Toast.LENGTH_LONG).show();
    }

    private class ConnectBT extends AsyncTask<Void, Void, Void> {
        private boolean ConnectSuccess = true;

        @Override
        protected void onPreExecute() {
            progress = ProgressDialog.show(LedControl.this, "Connecting...", "Please Wait!!!");
        }

        @Override
        protected Void doInBackground(Void... devices) {
            try {
                if (btSocket == null || !isBtConnected) {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();
                    BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);
                    btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();
                }
            } catch (IOException e) {
                ConnectSuccess = false;
            }

            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);

            if (!ConnectSuccess) {
                msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                finish();
            } else {
                msg("Connected");
                isBtConnected = true;
            }

            progress.dismiss();
        }
    }
}
