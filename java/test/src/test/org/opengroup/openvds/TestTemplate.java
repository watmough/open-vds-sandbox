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
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import static org.junit.Assert.*;
import static org.junit.Assume.assumeTrue;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TestTemplate {

    GlobalState globalState;

    @BeforeClass
    public static void setUpClass() {
    }

    @AfterClass
    public static void tearDownClass() {

    }

    @Before
    public void setUp() {
        this.globalState = OpenVDS.getGlobalState();
    }

    @After
    public void tearDown() {
    }
    
    public TestTemplate() {
    }

    /**
     * Undocumented test
     */
    @org.junit.Test
    public void test1() {
        assertNotNull(this.globalState);
        long dlcount = this.globalState.getBytesDownloaded(OpenOptions.ConnectionType.Other);
        assertTrue(dlcount == 0);
    }
    
}
