import os
from pathlib import Path

import pytest
import openvds

from segyimport_test_config import ImportExecutor, TempVDSGuard, tt_test_data_dir


@pytest.fixture
def crs_wkt() -> str:
    """
    A WKT to stuff into VDS metadata.  Not necessarily actually associated with the data, just something to
    test against.
    """
    return 'PROJCS["NAD27 / Wyoming East Central",GEOGCS["NAD27",DATUM["North_American_Datum_1927",SPHEROID["Clarke 1866",6378206.4,294.9786982138982,AUTHORITY["EPSG","7008"]],AUTHORITY["EPSG","6267"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4267"]],UNIT["US survey foot",0.3048006096012192,AUTHORITY["EPSG","9003"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",40.66666666666666],PARAMETER["central_meridian",-107.3333333333333],PARAMETER["scale_factor",0.999941177],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],AUTHORITY["EPSG","32056"],AXIS["X",EAST],AXIS["Y",NORTH]]'


@pytest.fixture
def output_vds() -> TempVDSGuard:
    return TempVDSGuard("import_poststack_test")


@pytest.fixture
def poststack_segy(tt_test_data_dir) -> str:
    return os.path.join(tt_test_data_dir, "3D_Stack", "ST0202R08_TIME.segy")


def test_crs_wkt(poststack_segy, output_vds, crs_wkt):
    ex = ImportExecutor()

    ex.add_args(["--crs-wkt", crs_wkt])
    ex.add_args(["--vdsfile", output_vds.filename])

    ex.add_arg(poststack_segy)

    result = ex.run()

    assert result == 0, ex.output()
    assert Path(output_vds.filename).exists()

    with openvds.open(output_vds.filename, "") as handle:
        layout = openvds.getLayout(handle)

        assert layout.isMetadataStringAvailable(
            openvds.KnownMetadata.surveyCoordinateSystemCRSWkt().category,
            openvds.KnownMetadata.surveyCoordinateSystemCRSWkt().name)

        value = layout.getMetadataString(
            openvds.KnownMetadata.surveyCoordinateSystemCRSWkt().category,
            openvds.KnownMetadata.surveyCoordinateSystemCRSWkt().name)
        assert value == crs_wkt
