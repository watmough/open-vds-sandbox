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

public class DownloadError {
	public int    ErrorCode;
	public String ErrorString;
	
	public DownloadError() {
		this.ErrorCode = 0;
		this.ErrorString = "";
	}
	
	public DownloadError(int errorCode, String errorString) {
		this.ErrorCode = errorCode;
		this.ErrorString = errorString;
	}

	DownloadError(Object[] arr) {
		assert(arr != null);
		assert(arr.length == 2);
		this.ErrorCode = (int)arr[0];
		this.ErrorString = (String)arr[1];
	}
}

