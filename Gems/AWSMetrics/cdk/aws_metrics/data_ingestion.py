"""
All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
its licensors.

For complete copyright and license terms please see the LICENSE at the root of this
distribution (the "License"). All use of this software is governed by the License,
or, if provided, by the license below or the license accompanying this file. Do not
remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
"""

from aws_cdk import (
    core,
    aws_apigateway as apigateway,
    aws_iam as iam,
    aws_kinesis as kinesis
)

from . import aws_metrics_constants

import json


class DataIngestion:
    """
    Create the service API via APIGateway and Kinesis data stream to ingest metrics events.
    """

    def __init__(self, stack: core.Construct, application_name: str) -> None:
        self._stack = stack

        # create the input Kinesis stream
        self._input_stream = kinesis.Stream(
            self._stack,
            id='InputStream',
            stream_name=f'{self._stack.stack_name}-InputStream',
            shard_count=1
        )

        apigateway_role = self._create_apigateway_role()

        # create the REST API resource
        self._rest_api = apigateway.SpecRestApi(
            self._stack,
            'RestApi',
            rest_api_name=f'{self._stack.stack_name}-RestApi',
            endpoint_export_name=f'{application_name}:RestApiEndpoint',
            api_definition=apigateway.ApiDefinition.from_asset('api_spec.json'),
            deploy_options=apigateway.StageOptions(
                method_options={
                    "/*/*": apigateway.MethodDeploymentOptions(
                        metrics_enabled=True,
                        logging_level=apigateway.MethodLoggingLevel.INFO
                    )
                },
                stage_name=aws_metrics_constants.APIGATEWAY_STAGE
            )
        )

        # read api_spec.json and replace the template variables
        with open("api_spec.json", "r") as api_spec:
            content = api_spec.read()
        content = content.replace("${ApiGatewayRoleArn}", apigateway_role.role_arn)
        content = content.replace("${InputStreamName}", self._input_stream.stream_name)
        api_definition = json.loads(content)

        # use escape hatches to override the API definitions with the actual resource information
        # https://docs.aws.amazon.com/cdk/latest/guide/cfn_layer.html
        cfn_rest_api = self._rest_api.node.default_child
        cfn_rest_api.add_property_override("Body", api_definition)
        cfn_rest_api.add_property_deletion_override("BodyS3Location")
        cfn_rest_api.add_property_override("FailOnWarnings", True)

        api_id_output = core.CfnOutput(
            self._stack,
            id='RestApiId',
            description='Service API Id for the analytics pipeline',
            export_name=f"{application_name}:RestApiId",
            value=self._rest_api.rest_api_id)

        stage_output = core.CfnOutput(
            self._stack,
            id='DeploymentStage',
            description='Stage for the REST API deployment',
            export_name=f"{application_name}:DeploymentStage",
            value=self._rest_api.deployment_stage.stage_name)

    def _create_apigateway_role(self) -> iam.Role:
        """
        Generate the IAM role for the REST API to integration with Kinesis.

        :return: The created IAM role.
        """
        api_gateway_put_kinesis_policy_document = iam.PolicyDocument(
            statements=[
                iam.PolicyStatement(
                    actions=[
                        "kinesis:PutRecord",
                        "kinesis:PutRecords"
                    ],
                    effect=iam.Effect.ALLOW,
                    resources=[
                        core.Fn.sub(
                            body="arn:${AWS::Partition}:kinesis:${AWS::Region}:${AWS::AccountId}:stream/${EventsStream}",
                            variables={
                                "EventsStream": self._input_stream.stream_name
                            }
                        )
                    ]
                )
            ]
        )

        apigateway_role = iam.Role(
            self._stack,
            id="ApiGatewayRole",
            role_name=f'{self._stack.stack_name}-ApiGatewayRole',
            assumed_by=iam.ServicePrincipal(
                service="apigateway.amazonaws.com"
            ),
            inline_policies={
                "ApiGatewayPutKinesisPolicy": api_gateway_put_kinesis_policy_document
            }
        )

        return apigateway_role

    @property
    def input_stream_arn(self) -> kinesis.Stream.stream_arn:
        return self._input_stream.stream_arn

    @property
    def input_stream_name(self) -> kinesis.Stream.stream_name:
        return self._input_stream.stream_name

    @property
    def rest_api_id(self) -> apigateway.RestApi.rest_api_id:
        return self._rest_api.rest_api_id

    @property
    def deployment_stage(self) -> str:
        return aws_metrics_constants.APIGATEWAY_STAGE

    @property
    def execute_api_arn(self) -> apigateway.RestApi.arn_for_execute_api:
        return self._rest_api.arn_for_execute_api(stage=aws_metrics_constants.APIGATEWAY_STAGE)