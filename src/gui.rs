mod repos;

use crate::repos::{
    get_config,
    get_name,
    fetch_check,
    Status::{self, *}
};

use iced::{Element, Subscription, application, Task};
use iced::widget::{Column, column, text};
use iced::time::{self, Duration};
use iced::futures::future::join_all;

use rayon::prelude::*;

use tokio;

#[derive(Debug)]
enum Event {
    StartCheck,
    FinishedCheck(Vec<Status>)
}

struct State {
    names: Vec<String>,
    repos: Vec<String>,
    status: Vec<Status>
}

impl State {
    fn default() -> Self {
        let repos = match get_config() {
            Ok(path) => path,
            Err(e) => panic!("{:?}", e)
        };

        let mut names: Vec<String> = vec![String::from("Unknow"); repos.len()];

        repos.par_iter()
             .zip(names.par_iter_mut())
             .for_each(|(repo_i, names_i)| 
        {
            *names_i = get_name(repo_i);
        }); 

        let mut status = vec![Unknow; names.len()];

        status.par_iter_mut()
        .zip(repos.par_iter())
        .for_each(|(status_i, repo_i)| {
            *status_i = fetch_check(repo_i);
        });

        State { 
            names: names.clone(),
            repos,
            status
        }
    }
}

async fn check_status(repos: Vec<String>) -> Vec<Status> {
    let tasks = repos.into_iter().map(|item| {
        tokio::spawn(async move {
            fetch_check(&item)
        })
    });

    let results = join_all(tasks).await;
    results.into_iter().filter_map(|res| res.ok()).collect()
}

fn update(state: &mut State, message: Event) -> Task<Event> {
    match message {
        Event::StartCheck => {
            println!("Start checking");
            Task::perform(
                check_status(state.repos.clone()),
                Event::FinishedCheck
            )
        },
        Event::FinishedCheck(res) => {
            println!("Checking done");
            state.status = res;
            Task::none()
        }
    }
}

fn subscription(_: &State) -> Subscription<Event> {
    time::every(Duration::from_secs(10))
          .map(|_| Event::StartCheck)
}

fn view(state: &State) -> Element<'_, Event> {
    let mut col: Column<'_, Event> = column![];

    for i in 0..state.names.len() {
        let name = &state.names[i];
        let repo: &String = &state.status[i].into();

        let content: String = format!("{}: {}", name.as_str(), repo);

        col = col.push(text(content));
    }
    col.into()
}

fn main() -> iced::Result {
    application(State::default, update, view)
        .subscription(subscription)
        .run()
}
