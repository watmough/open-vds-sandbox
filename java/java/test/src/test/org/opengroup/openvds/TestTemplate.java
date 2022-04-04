/*
 * Copyright 2022 The Open Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
package test.org.opengroup.openvds;

import org.opengroup.openvds.*;

import static org.testng.Assert.*;
import org.testng.annotations.*;

public class TestTemplate {

    GlobalState globalState;

    @BeforeClass
    public void setUpClass() {
        this.globalState = OpenVDS.getGlobalState();
    }

    @AfterClass
    public static void tearDownClass() {
    }

    public TestTemplate() {
    }

    /**
     * Undocumented test
     */
    @Test
    public void test1() {
        assertNotNull(this.globalState);
        long dlcount = this.globalState.getBytesDownloaded(OpenOptions.ConnectionType.Other);
        assertTrue(dlcount == 0);
    }
    
}
