package com.bart.udpclient;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.IOException;
import java.net.*;

public class MainActivity extends AppCompatActivity {
    private DatagramSocket udpSocket;
    private InetAddress serverAddress;
    private int port = 44445;

    public MainActivity() throws UnknownHostException, SocketException {
        serverAddress = InetAddress.getByName("192.168.178.67");
        udpSocket = new DatagramSocket(port);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btnSend = findViewById(R.id.Send);
        btnSend.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                new Thread(new Runnable() { // Network operations are not allowed on the main thread.
                    public void run() {
                        String sendText = "Super cool text.";
                        DatagramPacket p = new DatagramPacket( // Create packet with our text and destination.
                                sendText.getBytes(),
                                sendText.getBytes().length, serverAddress, port
                        );

                        try {
                            udpSocket.send(p); // Send packet.
                            appendText("Sent: " + sendText);
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }).start();
            }
        });
        startListener();
    }

    private void startListener() {
        // Network operations are not allowed on the main thread.
        new Thread(new Runnable() {
            public void run() {
                while (true) { // Infinite loop which listens for packets.
                    byte[] buf = new byte[256];
                    DatagramPacket packet = new DatagramPacket(buf, buf.length);
                    try {
                        udpSocket.receive(packet); // Blocks until a packet is received
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    String msg = new String(packet.getData()).trim(); // Trim removes leading and trailing white spaces.

                    appendText(packet.getAddress().getHostAddress() + ": " + msg);
                }
            }
        }).start();
    }

    private void appendText(final String text) {
        // This function is executed from a different thread than the main thread.
        // UI modifications need to be run on the main thread.

        Handler mainHandler = new Handler(Looper.getMainLooper()); // Get the handler linked to the main thread.
        Runnable runnable = new Runnable() { // Define the UI modification.
            @Override
            public void run() {
                TextView tv = findViewById(R.id.dataLog);
                tv.append(text + "\n");
            }
        };
        // Add it to the main thread execution queue so it will be run at the main thread.
        mainHandler.post(runnable);
    }
}
