/*
 * Copyright 2022 The Open Group
 * Copyright 2022 Bluware, Inc.
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

package org.opengroup.openvds;

public class UploadError {
	public String ObjectID;
	public int    ErrorCode;
	public String ErrorString;
	
	public UploadError() {
		this.ObjectID = "";
		this.ErrorCode = 0;
		this.ErrorString = "";
	}
	
	public UploadError(String objectID, int errorCode, String errorString) {
		this.ObjectID = objectID;
		this.ErrorCode = errorCode;
		this.ErrorString = errorString;
	}

	UploadError(Object[] arr) {
		assert(arr != null);
		assert(arr.length == 3);
		this.ObjectID = (String)arr[0];
		this.ErrorCode = (int)arr[1];
		this.ErrorString = (String)arr[2];
	}

}

