package org.opengroup.openvds;

import org.testng.annotations.Test;

import static org.testng.Assert.assertEquals;

public class VdsErrorTest {

    @Test
    void testVdsError() {
        VdsError error = new VdsError();

        int code = 43;
        String errorMessage = "Error message for error 43";

        error.setErrorCode(code);
        error.setMessage(errorMessage);

        int resCode = error.getErrorCode();
        String resMsg = error.getMessage();

        assertEquals(resCode, code);
        assertEquals(resMsg, errorMessage);
    }
}
