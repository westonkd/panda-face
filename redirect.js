$(document).ready(function(){
  localStorage.setItem('code', oauthCode());
  tokenRequest(tokenSuccess);
});

function tokenSuccess(responseText, xhr) {
  console.log(responseText);
}

function tokenRequest(callback) {
  var url = localStorage.getItem('base_url') + "/login/oauth2/token";
  var params = "grant_type=authorization_code" +
               "&client_id=" + localStorage.getItem('base_url') +
               "&client_secret=" + localStorage.getItem('user_key') +
               "&redirect_uri=" + "https://westonkd.github.io/panda-face/redirect" +
               "&code=" + localStorage.getItem('code');
  var xhr = new XMLHttpRequest();

  xhr.open('POST', url, true);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      callback(xhr.responseText, xhr);
    } else if (xhr.readyState == 4) {
      console.log("ERROR!");
      console.log(xhr);
    }
  };

  xhr.send(params);
}

function oauthCode() {
  var splitUrl = window.location.toString().split('?');
  if (splitUrl.length > 1 && splitUrl[1].indexOf('code') > -1) {
    queryHash = QueryStringToHash(splitUrl[1]);
    return queryHash.code;
  }

  return undefined;
}

//http://stackoverflow.com/questions/1131630/the-param-inverse-function-in-javascript-jquery
function QueryStringToHash(query) {
  var query_string = {};
  var vars = query.split("&");
  for (var i=0;i<vars.length;i++) {
    var pair = vars[i].split("=");
    pair[0] = decodeURIComponent(pair[0]);
    pair[1] = decodeURIComponent(pair[1]);
      // If first entry with this name
      if (typeof query_string[pair[0]] === "undefined") {
        query_string[pair[0]] = pair[1];
      // If second entry with this name
    } else if (typeof query_string[pair[0]] === "string") {
      var arr = [ query_string[pair[0]], pair[1] ];
      query_string[pair[0]] = arr;
      // If third or later entry with this name
    } else {
      query_string[pair[0]].push(pair[1]);
    }
  }
  return query_string;
};
