//! [Input Format CSV]
N/A,Voters,CatACan1,CatACan2,CatACan3,CatBCan1,CatBCan2
X,Jim,0,3,5,4,2
X,Sarah,5,2,2,1,4
X,Ted,3,0,0,5,0
//! [Input Format CSV]

//! [Input Format INI]
[Categories]
Category A = 3
Category B = 2

[General]
Seats = 2
//! [Input Format INI]

//! [Expected Results Format]
[
  [
    {
      "winner": "Sixth film",
      "qualifier": {
        "firstAdv": ["Sixth film", "Eighth film"],
        "secondAdv": []       
      }
    },
    {
      "winner": "Seventh film",
      "qualifier": {
        "firstAdv": ["Eighth film"],
        "secondAdv": ["Seventh film"]       
      }
    }
  ],
  [
    {
      "winner": "Fourth Director",
      "qualifier": {
        "firstAdv": ["Fourth Director"],
        "secondAdv": ["Fifth Director"]       
      }
    },
    {
      "winner": "Fifth Director",
      "qualifier": {
        "firstAdv": ["Fifth Director"],
        "secondAdv": ["Second Director"]       
      }
    }
  ]
]
//! [Expected Results Format]

//! [Calculator Options Format]
AllowTrueTies
CondorcetProtocol
//! [Calculator Options Format]