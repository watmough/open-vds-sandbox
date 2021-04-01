package org.opengroup.openvds;

import org.testng.annotations.Test;

import static org.testng.Assert.assertEquals;

public class MetaDataContainerTest {
    @Test
    void testMetadataContainer() {
        // creates object
        MetadataContainer container = new MetadataContainer();

        // set values
        int singleInt = 1337;
        container.setMetadataInt("CategoryInt", "intMetaData", singleInt);

        // get values
        int readSingleInt = container.getMetadataInt("CategoryInt", "intMetaData");

        // check equality
        assertEquals(singleInt, readSingleInt);
    }
}
