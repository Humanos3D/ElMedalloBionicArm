# Robotic Arm

Project for development of the robotic arm, and also our first Github project.  

## Github benefits

1. Code and designs can be version controlled using [Git](https://git-scm.com).  Git is the industry-standard for software version control and a very powerful tool.  It can have a bit of a learning curve (see [Getting Started with Git](https://git-scm.com)) but I don't think everyone needs to be fully versed.  The documentation and project management features can be used online without having to know what's under the hood.  Also note that we should not version control large binary files (CAD designs, etc.) using Git as the repository keeps a copy of **every** version of the file, and every computer with the repository gets all this information.  I.e., the repositories bloat quickly if you version control large files.  Also note that graphical tools such as [Sourcetree](https://www.sourcetreeapp.com) and/or [TortoiseGit](https://tortoisegit.org) can significantly simplify git usage.

1. A web-based interface but the code repository (or even the Wiki documentation repository) can be cloned and edited locally.  The data is all in the cloud, and so can be easily accessed from anywhere.

1. Github is a standard for sharing code.  If we want to open-source code and/or documentation, our information will be ready to go.  Once we have established non-profit status we can use both public and private repositories for free!

1. Github has a number of very useful project management tools.

   a. *Issues*. Tasks can be described, keywords associated, and responsibility assigned in the Issues tab.  Discussions of the task take place in the issue itself and are open to the team.  E-mail notifications can be enabled (or disabled) for every new comment on an issue.  You can also @ a team member in the discussion and they will automatically be added to the notifications.  In this manner the discussion of a topic is saved in an organized way, and can be searched at a later date.

   b. *Project Board*. The issues can be arranged into a [Kanban board](https://kanbanize.com/kanban-resources/getting-started/what-is-kanban-board/) (see the Project tab) so that the team can easily visualize priorities, next steps and responsibilities.
   
   c. *Milestones*. Milestones, including a date, can be set and issues associated.  Issues can then be filtered by milestone to see what is outstanding in order to achieve a milestone. 
   
1. We can use Github to house our documentation and make it easily available online.  While more sophisticated approaches exist (e.g., reStructuredText housed in the repository) I think the Github Wikis are adequate for us and easy to use.  The Github site holds a project Wiki where notes and documentation can be easily added.  The documentation is written in [Markdown](https://help.github.com/articles/basic-writing-and-formatting-syntax/), which is a text based format that can be easily used to include links, format text, add images, etc.

## Github Limitations

1. Git is not a streamlined communications tool (c.f. Slack).  Discussions can be viewed online, and notifications are sent via e-mail, but there is no mobile app, social media integration, etc.

1. Git is not a fully featured project management tool.  I've found the Issues structure more than adequate but there is no functionality for advanced features such as Gantt charts, resource planning, etc.
