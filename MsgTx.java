/*
 * @Author: FANZHEYANG zheyangfan793@gmail.com
 * @Date: 2026-07-16 10:47:42
 * @LastEditors: FANZHEYANG zheyangfan793@gmail.com
 * @LastEditTime: 2026-07-21 15:41:16
 * @FilePath: \BLE_all0307 (gnss和rdss实现）\MsgTx.java
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
package com.hcxtone.xtjd.data;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class MsgTx {
    /**
     * struct msg_tx{
     * unsigned char lf;//频点
     * unsigned char encode;//编码类型
     * unsigned char generation;//北⼆或北三
     * unsigned char reservied;
     * unsigned int dest_card;//⽬标卡号
     * unsigned int payload_len;
     * unsigned char payload[70];
     * };
     */

    public final byte lf = 1;
    public final byte encode;
    public final byte generation;
    public final byte reservied = 0;
    public final int dest_card;
    public String payload;
    private final int MAX_LEN = 1024;

    public MsgTx(int destCard, String payload, byte encode, byte generation) {
        dest_card = destCard;
        this.payload = payload;
        this.encode = encode;
        this.generation = generation;
    }

    public byte[] toByteArray() {
        try {
            byte[] originString = payload.getBytes("GB2312");
            final int strLen = originString.length;
            byte[] dstBytes;
            int msgLen = strLen;
            if (strLen > MAX_LEN) {
                dstBytes = new byte[MAX_LEN];
                System.arraycopy(dstBytes, 0, originString, 0, MAX_LEN);
                msgLen = MAX_LEN;
            } else {
                dstBytes = originString;
            }
            ByteBuffer bb = ByteBuffer.allocate(msgLen + 12);
            bb.order(ByteOrder.LITTLE_ENDIAN);

            bb.put(lf);
            bb.put(encode);
            bb.put(generation);
            bb.put(reservied);
            bb.putInt(dest_card);
//            if (payload.length() > 35) {
//                payload = payload.substring(0, 35);
//            }
            bb.putInt(msgLen);

            bb.put(dstBytes);
//            if (encode == 0) { // 汉字
//                String convertString = StringUtils.half2FullChange(payload);
//                byte[] convertBytes = convertString.getBytes("GB2312");
//                bb.put(convertBytes);
//            } else if (encode == 1) { // 代码
//                bb.put(originString);
//            } else { // 混码
////                bb.put(new byte[]{(byte) 0xA4});
//                bb.put(originString);
//            }
            return bb.array();
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }
}
