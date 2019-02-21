# Robotic Arm

Project for development of the robotic arm, and also our first Github project.  

## Github benefits

1. Code and designs can be version controlled using [Git](https://git-scm.com).  Git is the industry-standard for software version control and a very powerful tool.  It can have a bit of a learning curve (see [Getting Started with Git](https://git-scm.com)) but I don't think everyone needs to be fully versed.  The documentation and project management features can be used online without having to know what's under the hood.  

1. A web-based interface but the code repository (or even the Wiki documentation repository) can be cloned and edited locally.  The data is all in the cloud, and so can be easily accessed from anywhere.

1. Github is a standard for sharing code.  If we want to open-source code and/or documentation, our information will be ready to go.  Once we have established non-profit status we can use both public and private repositories for free!

1. Github has a number of very useful project management tools.

   a. *Issues*. Tasks can be described, keywords associated, and responsibility assigned in the Issues tab.  Discussions of the task take place in the issue itself and are open to the team.  E-mail notifications can be enabled (or disabled) for every new comment on an issue.  You can also @ a team member in the discussion and they will automatically be added to the notifications.  In this manner the discussion of a topic is saved in an organized way, and can be searched at a later date.

   b. *Project Board*. The issues can be arranged into a [Kanban board](https://kanbanize.com/kanban-resources/getting-started/what-is-kanban-board/) (see the Project tab) so that the team can easily visualize priorities, next steps and responsibilities.
   
   c. *Milestones*. Milestones, including a date, can be set and issues associated.  Issues can then be filtered by milestone to see what is outstanding in order to achieve a milestone. 
   
1. We can use Github to house our documentation and make it easily available online.  While more sophisticated approaches exist (e.g., reStructuredText housed in the repository) I think the Github Wikis are adequate for us and easy to use.  The Github site holds a project Wiki where notes and documentation can be easily added.  The documentation is written in [Markdown](https://help.github.com/articles/basic-writing-and-formatting-syntax/), which is a text based format that can be easily used to include links, format text, add images, etc.

## Github Limitations

1. Git is not a streamlined communications tool (c.f. Slack).  Discussions can be viewed online, and notifications are sent via e-mail, but there is no mobile app, social media integration, etc.

1. Git is not a fully featured project management tool.  I've found the Issues structure more than adequate but there is no functionality for advanced features such as Gantt charts, resource planning, automated reminders, etc.

## Tips and Recommendations 

1. We should not use Git/Github to version control large binary files (CAD designs, etc.) as the repository keeps a copy of **every** version of the file, and every computer with the repository gets all this information.  I.e., the repositories bloat quickly if you version control large files.  We should continue using Google Drive for this sort of data.

1. We should set up an admin repository.  This can hold *Issues* for cross project work (e.g., the shopping list), the Wiki can be used to record meeting minutes, etc. We can also move this Github primer to that repository.  This repository should be private (probably some/most others too).

1. Do we want to also track progress on prosthesis assembly etc. on Github (i.e., the list that's currently on the whiteboard)? If we have no complaints with the whiteboard approach, then maybe not.  But it could be a good place to order the work, keep track of setbacks, etc.

1. Many users who deal with the code repository may want to install graphical tools such as [Sourcetree](https://www.sourcetreeapp.com) and/or [TortoiseGit](https://tortoisegit.org) on their computers.  This can significantly simplify git usage, especially if you're just getting started.

1. To the extent possible, we should keep all our work in the Github and Google Drive framework, with documentation living in the Github wikis.  For example, we may want to include our user survey here - version control would be useful and it'll put the document in a location where everyone can comment and/or edit (but all edits can be rolled back thanks to version control). **Duplicate data (e.g., a list kept on a white board and in Github) almost always end up causing problems - everyone needs to agree where a piece of information lives**.

1. We should use *Issues* to keep track of all of our to-do work.  The project Kanban board is a great way to get a snapshot of overall progress and activity, but can also be easily transformed into your own personal to-do list by filtering by assignee.

1. We can include images in our documentation but it's a little bit trickier.  My recommendation is
   * Check out a copy of the Wiki repository (e.g., https://github.com/enable-medellin/robotic-arm.wiki.git; this link is given at the bottom right of the Wiki page.
   * Add your desired image (PNG, JPG, GIF, PSD, and SVG should all work) to a subdirectory in the repository called `images`.
   * Commit your changes both locally and to the remote repository (i.e., do git `commit` and `push` operations)
   * The images can then be displayed in your Wiki pages by using HTML syntax to link to them on the web.  E.g., see [this page](https://github.com/enable-medellin/robotic-arm/wiki/Protesis-Avanzadas-EMG-Sensor), where I use the command `<img src="https://raw.githubusercontent.com/wiki/enable-medellin/robotic-arm/images/pa-emg-front.jpg" width="300">`.  You'll need to swap in your file name and potentially change the `robotic-arm` part of the address if you're working in a different repository.  Also note that the image size can be controlled (done using the `width="300"` option here.
   
1. People should generally add their own issues as needs arise.  Don't be shy of assigning tasks to other people - they can always come talk to you if they can't do it or are short on time.  If granular tasking is self managed it will free up Adam's time, and he can just do a weekly review and redirect.

1. Everyone gets signed up for Github!
