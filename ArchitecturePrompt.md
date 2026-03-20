As an expert technical architect,  create a narrative comprehensive architectural document along with mermaid diagrams for this codebase in this project. Write to a file named ArchitectureDocument in 
Also, I need to reverse engineer the codebase in this project and create class diagrams and object relationship diagrams. Please help me analyze this codebase systematically. 
  Please follow these steps: 
1. Identify all classes, their attributes, and methods 
2. Map inheritance relationships between classes 
3. Document composition and aggregation relationships 
4. Identify key interfaces and their implementations 
5. Analyze dependency patterns between components 
6. Create a UML class diagram showing these relationships 
7. Generate an object relationship diagram highlighting runtime interactions 
8. Identify any design patterns present in the code 
please use a format that's easy to understand with clear labels for relationships. If possible, organize the diagrams by logical components or modules. Where the code is ambiguous or has implicit relationships, please highlight these areas and provide your best interpretation based on the available code.

Append this to the ArchitectureDocument.md in 
 
I need to extract information about upstream and downstream systems that interact with  codebase of this project. 
 
Please analyze the code to map out all external system dependencies and integrations: Please follow these steps: 
 1. Identify all external API calls, service integrations, and third-party dependencies  - Include URLs, endpoints, or service names where available  - Note authentication methods used for these connections 
2. Map all upstream systems (systems that send data to this application)  - Identify data ingestion points (APIs, message queues, webhooks, file imports)  - Document the type and format of data received  - Note any data transformation that occurs upon receipt 
3. Map all downstream systems (systems that receive data from this application)  - Identify outbound data flows (API calls, message publishing, file exports)  - Document the type and format of data sent  - Note any data transformation that occurs before sending 
4. Analyze database interactions  - Identify all database connections and their purposes  - Note if databases are acting as integration points with other systems 
5. Identify any message brokers, event buses, or queuing systems  - Map the publishers and subscribers  - Document the event/message types and their flows 
6. Create a system context diagram showing:  - The legacy system at the center  - All upstream systems with inbound data flows  - All downstream systems with outbound data flows  - The nature of each integration (real-time, batch, etc.) 
7. Document any configuration files that contain integration details  - Include environment variables, config files, or hardcoded connection strings 
8. Note potential integration risks or vulnerabilities
- Deprecated APIs or services  
- Lack of error handling  
- Single points of failure 
If you notice any implicit or ambiguous system dependencies, please highlight these and provide your best interpretation based on the code context.
Append this to the ArchitectureDocument.md in 


As a business analyst, your goal is to generate detailed user stories based on the functionality and business logic identified in the codebase of this project. 
 
These user stories should provide sufficient detail and clarity for developers to write code directly. 
Additionally, the user stories should be formatted in a way that can be easily exported and uploaded to Jira.
 
To create the user stories and export them for Jira, follow these steps:
 
1. Identify the key features and modules of the legacy application:

   - Review the architecture documentation and codebase structure

   - Analyze the business workflows and user interactions

   - Break down the application into smaller, manageable features or modules
 
2. For each feature or module:

   - Define the user story template: "As a [user role], I want to [goal/desire], so that [benefit/value]"

   - Clearly specify the user role, their goal, and the benefit they expect to achieve

   - Provide a detailed description of the desired functionality

   - Include any specific business rules, validations, or constraints that apply
 
3. Breakdown the user story into detailed acceptance criteria:

   - Define the specific scenarios and steps required to fulfill the user story

   - Specify the input data, expected output, and any edge cases to consider

   - Describe the user interface interactions and navigation flow

   - Include any error handling or exceptional scenarios that need to be addressed

   - Provide examples or mockups to illustrate the desired behavior
 
4. Define the data model and data interactions:

   - Identify the entities, attributes, and relationships involved in the user story

   - Specify any data validations, formatting, or business rules related to the data

   - Describe how the data should be stored, retrieved, and manipulated

   - Provide examples of data inputs and expected outputs
 
5. Specify the API contracts and integration points:

   - Define the API endpoints, request/response formats, and any authentication requirements

   - Describe the expected behavior and error handling for each API endpoint

   - Specify any third-party integrations or external dependencies
 
6. Outline the non-functional requirements:

   - Define the performance expectations, such as response times or throughput

   - Specify any scalability requirements, such as handling a certain number of concurrent users

   - Describe the security measures to be implemented, such as authentication, authorization, or data encryption

   - Specify any usability or accessibility requirements
 
7. Provide test scenarios and acceptance criteria:

   - Define the test cases or scenarios that cover the different aspects of the user story

   - Specify the expected results and any edge cases to be tested

   - Provide test data or mocks to be used during testing
 
8. Format the user stories for Jira export:

   - Use a consistent template for each user story, including fields such as Summary, Description, Acceptance Criteria, and Story Points

   - Ensure that the user stories are written in a clear and concise manner

   - Use Jira's formatting conventions, such as using asterisks (*) for bullet points and double square brackets ([[]]) for links

   - Include any relevant attachments, such as mockups or diagrams, and provide clear references to them in the user story description
 
9. Export the user stories to a file format compatible with Jira:

   - Use a tool or script to convert the user stories into a format that can be imported into Jira, such as CSV or JSON

   - Ensure that the exported file includes all the necessary fields and maintains the formatting of the user stories

   - Verify that the exported file can be successfully imported into Jira without any data loss or formatting issues
 
10. Collaborate with the development team:

    - Review the exported user stories with the development team to ensure clarity and feasibility

    - Gather feedback and address any technical concerns or limitations

    - Iterate on the user stories based on the feedback received
 
11. Import the user stories into Jira:

    - Follow Jira's import process to upload the exported file containing the user stories

    - Map the fields from the exported file to the corresponding fields in Jira

    - Verify that the user stories are correctly imported and maintain their formatting and attachments

    - Assign the user stories to the appropriate project and team members in Jira
 
By following this prompt, you can generate detailed user stories that are formatted and exported in a way compatible with Jira. The exported file can be easily imported into Jira, allowing the development team to access and work on the user stories directly within their project management tool.
 
Remember to collaborate closely with the development team throughout the process to ensure that the user stories are well-understood and aligned with the technical implementation. Regular communication and iteration will help refine the user stories and ensure a smooth transition from requirements to development tasks in Jira.
 
Feel free to adapt and expand upon this prompt based on the specific requirements and tools used in your project. The key is to provide a seamless integration between the user story generation process and the project management tool, enabling efficient collaboration and tracking of development progress.
Append this to the ArchitectureDocument.md in 

