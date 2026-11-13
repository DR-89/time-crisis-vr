package org.timecrisis.quest;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.util.Log;
import java.util.Collections;
import java.util.UUID;

/**
 * Optional WonderFitter BLE gun (M17 pistol, M416 rifle). The gun notifies on FFF1:
 *   shot     5B SS 07 07 CK 5D   (SS = sequence, sent three times per shot)
 *   magazine 5C MM CK 5D         (MM 00 = removed, 01 = inserted)
 * where CK is the low byte of the sum of the preceding bytes. Shots and magazine
 * changes are forwarded to the native host; without a gun nothing changes.
 */
final class WonderFitterGun {
    private static final String TAG = "TCVR-WF";
    private static final UUID SERVICE = UUID.fromString("0000fff0-0000-1000-8000-00805f9b34fb");
    private static final UUID NOTIFY = UUID.fromString("0000fff1-0000-1000-8000-00805f9b34fb");
    private static final UUID CCCD = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private static native void nativeShot();
    /** -1 = no gun, 0 = magazine removed (take cover), 1 = magazine inserted. */
    private static native void nativeMagazine(int state);

    private static Context context;
    private static BluetoothAdapter adapter;
    private static BluetoothGatt gatt;
    private static final Handler main = new Handler(Looper.getMainLooper());
    private static boolean scanning;
    private static int lastSequence = -1;

    private WonderFitterGun() {}

    /** Releases the gun for another app (Tremu); start() reconnects when this game resumes. */
    static void stop() {
        if (adapter == null) return;
        try {
            if (scanning) adapter.getBluetoothLeScanner().stopScan(scanCallback);
            if (gatt != null) { gatt.disconnect(); gatt.close(); }
        } catch (SecurityException | NullPointerException e) {
            android.util.Log.w(TAG, "Cannot release the gun: " + e);
        }
        scanning = false;
        gatt = null;
        adapter = null;
    }

    static void start(Context ctx) {
        if (adapter != null) return;
        context = ctx.getApplicationContext();
        BluetoothManager manager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        adapter = manager == null ? null : manager.getAdapter();
        if (adapter == null || !adapter.isEnabled()) {
            Log.i(TAG, "Bluetooth unavailable; WonderFitter gun disabled");
            adapter = null;
            return;
        }
        scan();
    }

    private static void scan() {
        if (scanning || gatt != null) return;
        try {
            ScanFilter filter = new ScanFilter.Builder().setServiceUuid(new ParcelUuid(SERVICE)).build();
            ScanSettings settings = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();
            adapter.getBluetoothLeScanner().startScan(Collections.singletonList(filter), settings, scanCallback);
            scanning = true;
            Log.i(TAG, "Scanning for a WonderFitter gun");
        } catch (SecurityException | NullPointerException e) {
            Log.w(TAG, "Cannot scan: " + e);
        }
    }

    private static final ScanCallback scanCallback = new ScanCallback() {
        @Override public void onScanResult(int type, ScanResult result) {
            if (gatt != null) return;
            try {
                adapter.getBluetoothLeScanner().stopScan(this);
                scanning = false;
                BluetoothDevice device = result.getDevice();
                Log.i(TAG, "Connecting to " + device.getAddress() + " " + device.getName());
                gatt = device.connectGatt(context, false, gattCallback, BluetoothDevice.TRANSPORT_LE);
            } catch (SecurityException e) {
                Log.w(TAG, "Cannot connect: " + e);
            }
        }
    };

    private static final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override public void onConnectionStateChange(BluetoothGatt g, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                g.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "Gun disconnected");
                g.close();
                gatt = null;
                notifyMagazine(-1);
                main.postDelayed(WonderFitterGun::scan, 2000);
            }
        }

        @Override public void onServicesDiscovered(BluetoothGatt g, int status) {
            BluetoothGattService service = g.getService(SERVICE);
            BluetoothGattCharacteristic ch = service == null ? null : service.getCharacteristic(NOTIFY);
            BluetoothGattDescriptor cccd = ch == null ? null : ch.getDescriptor(CCCD);
            if (cccd == null) {
                Log.w(TAG, "Gun has no FFF1 notifications");
                return;
            }
            g.setCharacteristicNotification(ch, true);
            cccd.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            g.writeDescriptor(cccd);
        }

        @Override public void onDescriptorWrite(BluetoothGatt g, BluetoothGattDescriptor d, int status) {
            Log.i(TAG, "Gun ready (status " + status + ")");
            // The gun only reports magazine changes, so assume it starts loaded.
            notifyMagazine(1);
        }

        @Override public void onCharacteristicChanged(BluetoothGatt g, BluetoothGattCharacteristic ch) {
            parse(ch.getValue());
        }
    };

    private static void parse(byte[] data) {
        if (data == null) return;
        int i = 0;
        while (i < data.length) {
            int length = (data[i] & 0xFF) == 0x5B ? 6 : (data[i] & 0xFF) == 0x5C ? 4 : 0;
            if (length == 0 || i + length > data.length) { i++; continue; }
            int sum = 0;
            for (int k = i; k < i + length - 2; k++) sum += data[k] & 0xFF;
            if ((sum & 0xFF) != (data[i + length - 2] & 0xFF) || (data[i + length - 1] & 0xFF) != 0x5D) { i++; continue; }
            if (length == 6) {
                int sequence = data[i + 1] & 0xFF;
                if (sequence != lastSequence) {
                    lastSequence = sequence;
                    try { nativeShot(); } catch (UnsatisfiedLinkError ignored) {}
                }
            } else {
                notifyMagazine(data[i + 1] == 0 ? 0 : 1);
                Log.i(TAG, data[i + 1] == 0 ? "Magazine out: cover" : "Magazine in");
            }
            i += length;
        }
    }

    private static void notifyMagazine(int state) {
        try { nativeMagazine(state); } catch (UnsatisfiedLinkError ignored) {}
    }
}
