// API HELPER
function ApiHelper(baseUrl, token) {
  this.baseUrl = baseUrl;
  this.token = token;
  this.defaultText = "No Assignment";
  this.defaultTime = "NA";
  
  if (!this.token) {
    this.sendError("Please authorize");
  }
}

ApiHelper.prototype.fetchRespone = function(endpoint, type) {
  var url = this.baseUrl + endpoint;
  //navigator.onLine
  if (navigator.onLine) {
    this.xhrRequest(url, type, this.handleResponse);
  } else {
    var data = localStorage.getItem('assignment_data');
    if (data) {
      this.handleResponse(data, {}, this);
    } else {
      this.sendError("No Connection");
    }
  }
};

ApiHelper.prototype.sendError = function(message) {
  var dictionary = {
    'FIRST_ASSIGN': 'Error',
    'FIRST_DUE': message,
    'SECOND_ASSIGN': '',
    'SECOND_DUE': '',
    'FIRST_POINTS': '0',
    'SECOND_POINTS': '0',
    'COMPLETE': 1,
    'TOTAL': 1
  };
  Pebble.sendAppMessage(dictionary, this.successHandler, this.errorHandler);
};

ApiHelper.prototype.handleResponse = function(responseText, xhr, self) {
  localStorage.setItem('assignment_data', responseText);
  var data = JSON.parse(responseText);
  
  if (data.errors) {
    Pebble.openURL('https://westonkd.github.io/panda-face/');
    self.sendError('reauthorize');
  }
  
  var nextTwo = [];
  
  // Grab the next two assignments
  for (var i = 0; i < data.length && nextTwo.length < 2; i++) {
    if (data[i].type === 'assignment' && 
        !data[i].assignment.has_submitted_submissions) {
      nextTwo.push(data[i]);
    }
  }
  
  // Populate dictionary
  var dictionary = {
    'FIRST_ASSIGN': nextTwo.length > 0 ? nextTwo[0].title : this.defaultText,
    'FIRST_DUE': nextTwo.length > 0 ? self.timeUntilDue(nextTwo[0]) : self.defaultTime,
    'FIRST_POINTS': nextTwo.length > 0 ? nextTwo[0].assignment.points_possible.toString() :'0',
    'SECOND_ASSIGN': nextTwo.length > 1 ? nextTwo[1].title : this.defaultText,
    'SECOND_DUE': nextTwo.length > 0 ? self.timeUntilDue(nextTwo[1]) : self.defaultTime,
    'SECOND_POINTS': nextTwo.length > 0 ? nextTwo[1].assignment.points_possible.toString() : '0',
    'TOTAL': data.length,
    'COMPLETE': self.completeAssignments(data)
  };
  
  Pebble.sendAppMessage(dictionary, self.successHandler, self.errorHandler);
};

ApiHelper.prototype.timeUntilDue = function(assignment) {
  var dueDate = new Date(assignment.assignment.due_at).getTime();
  var currentDate = new Date(Date.now()).getTime();
  
  var time = (dueDate - currentDate) / 60000;
  var label = "Mins.";
  
  if (time > 60) {
    time = time / 60;
    label = "Hrs.";
  }
  
  return Math.round(time * 10) / 10 + ' ' + label;
};

ApiHelper.prototype.errorHandler = function(e) {
  console.log("Error sending response to Pebble.");
  console.log(e);
};

ApiHelper.prototype.xhrRequest = function(url, type, callback) {
    var xhr = new XMLHttpRequest();
    var self = this;
    xhr.onload = function () {
      callback(this.responseText, xhr, self);
    };
    xhr.open(type, url);
    xhr.setRequestHeader("Authorization","Bearer " + this.token);
    xhr.send();
};

ApiHelper.prototype.completeAssignments = function(assignments) {
  var complete = 0;
  for (var i = 0; i < assignments.length; i++) {
    if (assignments[i].type === 'assignment' && assignments[i].assignment.has_submitted_submissions) {
      complete++;
    }
  }
  return complete;
};


// PEBBLE EVENTS AND HELPERS
function tokenRequest(code) {
  var tokenUrl = localStorage.getItem('base_url') + "/login/oauth2/token";
  var formData = "grant_type=authorization_code" +
                "&client_id=" + localStorage.getItem('client_id') +
                "&client_secret=" + localStorage.getItem('client_secret') +
                "&redirect_uri=" + localStorage.getItem('redirect_uri') +
                "&code=" + code;
  console.log("FormData: " + formData);
  console.log("url: " + tokenUrl);
  
  var xhr = new XMLHttpRequest();
  xhr.open("POST", tokenUrl, true);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhr.onreadystatechange = function() {
    if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        localStorage.setItem('access_token',JSON.parse(xhr.responseText).access_token);
        console.log(JSON.parse(xhr.responseText).access_token);
      } else {
        console.log("Error:" + xhr.status);
        console.log(xhr.responseText);
      }  
    }
  };
  xhr.send(formData);
}

function getUpcomingEvents() {
  var baseUrl = localStorage.getItem('base_url');
  var token = localStorage.getItem('access_token');
  var apiFilter = new ApiHelper(baseUrl, token);
  if (token && baseUrl) {
    console.log("There is a token: " + token);
    apiFilter.fetchRespone("/api/v1/users/self/upcoming_events", "GET");
  }
}

Pebble.addEventListener('ready', 
  function(e) {
    getUpcomingEvents();
  }
);

Pebble.addEventListener('appmessage',
  function(e) {
    getUpcomingEvents();
  }                     
);

Pebble.addEventListener("showConfiguration", function() {
  Pebble.openURL('https://westonkd.github.io/panda-face/');
});

Pebble.addEventListener('webviewclosed', function(e) {
  var data = JSON.parse(e.response);
  // Initial info
  if (data.return_type === "app_info") {
    // Store needed data
    localStorage.setItem('client_id', data.client_id);
    localStorage.setItem('client_secret', data.client_secret);
    localStorage.setItem('redirect_uri', data.redirect_uri);
    localStorage.setItem('base_url', data.base_url);
    
    // Get the code
    Pebble.openURL(data.login_url);
    //Code
  } else if (data.return_type === "code") {
    tokenRequest(data.code);
  }
});