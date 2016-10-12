function ApiHelper(baseUrl, token) {
  this.baseUrl = baseUrl;
  this.token = token;
  this.defaultText = "No Assignment";
  this.defaultTime = "NA";
}

ApiHelper.prototype.fetchRespone = function(endpoint, type) {
  var url = this.baseUrl + endpoint;
  if (navigator.onLine) {
    console.log("online");
    this.xhrRequest(url, type, this.handleResponse);
  } else {
    var data = localStorage.getItem('assignment_data');
    this.handleResponse(data, {}, this);
  }
};

ApiHelper.prototype.handleResponse = function(responseText, xhr, self) {
  localStorage.setItem('assignment_data', responseText);
  var data = JSON.parse(responseText);
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
    'SECOND_ASSIGN': nextTwo.length > 1 ? nextTwo[1].title : this.defaultText,
    'SECOND_DUE': nextTwo.length > 0 ? self.timeUntilDue(nextTwo[1]) : self.defaultTime,
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

ApiHelper.prototype.banana = function(one, two) {
  var dictionary = {
    'FIRST_ASSIGN': "one",
    'FIRST_DUE': "Two",
    'SECOND_ASSIGN': "three",
    'SECOND_DUE': "four"
  };
  
  Pebble.sendAppMessage(dictionary, this.successHandler, this.errorHandler);
};



function getUpcomingEvents() {
  var baseUrl = "http://wdransfield.instructure.com";
  var token = "9336~4Q7vauyL6JxKma2JsE9dcWpoZb0p79f4rBHx9eD9KYvgpoKxLxgCWNTGX6W0CadA";
  var apiFilter = new ApiHelper(baseUrl, token);
  apiFilter.fetchRespone("/api/v1/users/self/upcoming_events", "GET");
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